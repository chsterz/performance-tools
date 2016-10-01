#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <utility>

#include <QItemSelection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMainWindow>
#include <QString>

#include "eventlistmodel.h"
#include "measuermentrunmodel.h"
#include "measurement.h"
#include "regressionmodel.h"
#include "ttestmodel.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  void measureOnce();
  void nodesCorrelation(EventsGroup events);
  ~MainWindow();

  QStringList measureEvents(QString command);

  std::vector<float> extractAverages();

signals:
  void progressChanged(int progress);

public slots:
  void showIndexes(const QModelIndex& clicked);

protected:

  QJsonArray m_coreJson;
  QJsonArray m_uncoreJson;

  EventListModel *m_coreEventsModel;
  EventListModel *m_uncoreEventsModel;

  MeasuermentRunModel *m_measurements;

  TTestModel *m_coreTtestModel;
  TTestModel *m_uncoreTtestModel;

  RegressionModel *m_coreRegressionModel;
  RegressionModel *m_uncoreRegressionModel;


  std::vector<MeasurementRun> m_measurementRuns;

  QString buildCommand(int eventIndex, QStringList eventRegistersz);
  void storeResults(EventsGroup selection, int eventIndex, QStringList results, QStringList eventRegisters, MeasurementRun &currentMeasurement);
  QStringList getEventRegistersForSelection(EventsGroup selection);
  void setColumnWidths();
  bool checkPerfInstalled();

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
