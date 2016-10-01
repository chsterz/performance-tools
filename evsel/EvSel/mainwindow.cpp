#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFile>

#include <assert.h>
#include <iostream>


///////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->progressBar->hide();
  ui->statusBar->hide();
  ui->threadsToEventsButton->setShortcut(tr("Ctrl+Space"));

  auto loadJson = [&](QString filename)
  {
    QFile file(filename);
    if( !file.open(QIODevice::ReadOnly) )
    {
      qDebug() << "Could not open Event Specification File";
      exit(EXIT_FAILURE);
    }
   return QJsonDocument::fromJson(file.readAll()).array();
  };

  m_coreJson = loadJson("../SKL/Skylake_core_V24.json");
  m_coreEventsModel = new EventListModel(m_coreJson, m_measurementRuns, EventsGroup::CORE, this);
  m_uncoreJson = loadJson("../SKL/Skylake_uncore_V24.json");
  m_uncoreEventsModel = new EventListModel(m_uncoreJson, m_measurementRuns, EventsGroup::UNCORE, this);
  m_measurements = new MeasuermentRunModel(m_measurementRuns, this);


  ui->coreEventsView->setModel(m_coreEventsModel);
  ui->uncoreEventsView->setModel(m_uncoreEventsModel);
  ui->measuerementView->setModel(m_measurements);
  ui->measuerementView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  setColumnWidths();

  QObject::connect(ui->threadsToEventsButton, &QPushButton::clicked,
                   [this](){measureOnce();});
  QObject::connect(this, &MainWindow::progressChanged,
                   [this](int progress){ui->progressBar->setValue(progress);});
  QObject::connect(ui->measuerementView, &QListView::clicked,
                   [this](const QModelIndex& clicked){this->showIndexes(clicked);});
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::showIndexes(const QModelIndex& clicked)
{
  auto selected = ui->measuerementView->selectionModel()->selectedIndexes();

  //Show Events
  if( selected.size() == 1 )
  {
    ui->coreEventsView->setModel(m_coreEventsModel);
    m_coreEventsModel->setMeasurementIndex(clicked.row());
    ui->coreEventsView->repaint();
    ui->uncoreEventsView->setModel(m_uncoreEventsModel);
    m_uncoreEventsModel->setMeasurementIndex(clicked.row());
    ui->uncoreEventsView->repaint();
  }
  //Show TTest
  else if ( selected.size() == 2 )
  {
    auto firstInList = std::min(selected.first().row(), selected.last().row());
    auto secondInList = std::max(selected.first().row(), selected.last().row());

    m_coreTtestModel = new TTestModel(m_coreJson,
                                  m_measurementRuns[firstInList],
                                  m_measurementRuns[secondInList],
                                  EventsGroup::CORE,
                                  this);
    m_uncoreTtestModel = new TTestModel(m_uncoreJson,
                                  m_measurementRuns[firstInList],
                                  m_measurementRuns[secondInList],
                                  EventsGroup::UNCORE,
                                  this);

    ui->coreEventsView->setModel(m_coreTtestModel);
    ui->uncoreEventsView->setModel(m_uncoreTtestModel);
  }
  //Show Regression
  else
  {
    std::vector<MeasurementRun*> regressionData = {};
    auto selectionVector = selected.toVector();
    qSort(selectionVector.begin(), selectionVector.end(), [](auto a, auto b){return a.row() < b.row();});
    for (auto index : selectionVector)
    {
      regressionData.push_back(&m_measurementRuns[index.row()]);
    }

    m_coreRegressionModel = new RegressionModel(m_coreJson,
                                                regressionData,
                                                EventsGroup::CORE,
                                                this);
    m_uncoreRegressionModel = new RegressionModel(m_uncoreJson,
                                                  regressionData,
                                                  EventsGroup::UNCORE,
                                                  this);

    ui->coreEventsView->setModel(m_coreRegressionModel);
    ui->coreEventsView->resizeColumnToContents(5);
    ui->uncoreEventsView->setModel(m_uncoreRegressionModel);
    ui->coreEventsView->resizeColumnToContents(5);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


QString MainWindow::buildCommand(int eventIndex, QStringList eventRegisters)
{
  QString command = "perf stat -x, -e";
  for(int registerIndex=eventIndex; registerIndex < std::min(eventIndex+4, eventRegisters.size()); registerIndex++)
  {
    command += eventRegisters[registerIndex] + ",";
  }
  command = command.left(command.length() - 1);

  return command;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::measureOnce()
{
  if(not checkPerfInstalled())
    return;

  ui->progressBar->show();
  ui->progressBar->repaint();

  MeasurementRun currentMeasurement(ui->commandEdit->text());

  auto measureSelection = [&](EventsGroup selection)
  {
    QStringList eventRegisters = getEventRegistersForSelection(selection);
      for(int eventIndex = 0; eventIndex < eventRegisters.size(); eventIndex += 4)
    {
    QString command = buildCommand(eventIndex, eventRegisters);
    command += " " + currentMeasurement.command();
    emit progressChanged((eventIndex/(float)eventRegisters.size()) * 100.0);
      for(int sampleIndex = 0; sampleIndex < ui->repetitions->value(); sampleIndex++)
      {
        QStringList results = measureEvents(command);
        if(results[0]=="Workload failed: No such file or directory")
        {
          ui->progressBar->hide();
          return;
        }
        storeResults(selection, eventIndex, results, eventRegisters, currentMeasurement);
      }
    }
  };

  measureSelection(EventsGroup::CORE);
  measureSelection(EventsGroup::UNCORE);

  m_measurementRuns.push_back(currentMeasurement);
  ui->measuerementView->setCurrentIndex(m_measurements->index(m_measurementRuns.size()-1));
  m_coreEventsModel->setMeasurementIndex(m_measurementRuns.size()-1);
  m_uncoreEventsModel->setMeasurementIndex(m_measurementRuns.size()-1);

  ui->coreEventsView->repaint();
  ui->uncoreEventsView->repaint();
  m_measurements->dataChanged();
  ui->measuerementView->repaint();
  ui->progressBar->hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QStringList MainWindow::measureEvents(QString command)
{
  QProcess process;
  process.setReadChannel(QProcess::StandardError);
  process.start(command);
  process.waitForFinished();
  QStringList results;
  results = QString(process.readAll()).split('\n');
  return results;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::storeResults(EventsGroup selection, int eventIndex, QStringList results, QStringList eventRegisters, MeasurementRun &currentMeasurement)
{
  for(int registerIndex=eventIndex; registerIndex < std::min(eventIndex+4,eventRegisters.size()); registerIndex++)
  {
    QStringList singleMeasurement = results.front().split(',');
    float value= singleMeasurement[0].toFloat();
    results.pop_front();
    currentMeasurement.data(selection, registerIndex) << value;
    currentMeasurement.data(selection, registerIndex).repetitions++;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QStringList MainWindow::getEventRegistersForSelection(EventsGroup selection)
{
  QStringList eventRegisters;
  if(selection == EventsGroup::CORE)
  {
    std::cout << "Threads Correlation on Core" << std::endl << std::flush;
    eventRegisters = m_coreEventsModel->getAllRegisters();
  }
  if(selection == EventsGroup::UNCORE)
  {
    std::cout << "Threads Correlation on UnCore" << std::endl << std::flush;
    eventRegisters = m_uncoreEventsModel->getAllRegisters();
  }
  return eventRegisters;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setColumnWidths()
{
  ui->coreEventsView->setColumnWidth(0, 50);
  ui->coreEventsView->setColumnWidth(1, 50);
  ui->coreEventsView->setColumnWidth(2, 200);
  ui->coreEventsView->setColumnWidth(3, 700);
  ui->coreEventsView->setColumnWidth(4, 150);
  ui->coreEventsView->setColumnWidth(5, 150);
  ui->uncoreEventsView->setColumnWidth(0, 50);
  ui->uncoreEventsView->setColumnWidth(1, 50);
  ui->uncoreEventsView->setColumnWidth(2, 200);
  ui->uncoreEventsView->setColumnWidth(3, 700);
  ui->uncoreEventsView->setColumnWidth(4, 150);
  ui->uncoreEventsView->setColumnWidth(5, 150);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool MainWindow::checkPerfInstalled()
{
  QFile perfExecutable("/usr/bin/perf");
  if(not perfExecutable.exists() )
  {
    QMessageBox message(this);
    message.setIcon(QMessageBox::Critical);
    message.setText("Could not perform measurement: 'perf' not installed.");
    message.exec();
    return false;
  }
  else
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
  delete ui;
}
