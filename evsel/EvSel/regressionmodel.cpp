#include "regressionmodel.h"

#include <QJsonObject>
#include <QColor>

RegressionModel::RegressionModel(QJsonArray& events,
                                 std::vector<MeasurementRun*> measurementRuns,
                                 EventsGroup coreOrUncore,
                                 QObject* parent = nullptr)
  : QAbstractTableModel(parent),
    m_events(events),
    m_CoreOrUncore(coreOrUncore),
    m_measurementRuns(measurementRuns)
{}

///////////////////////////////////////////////////////////////////////////////////////////////////

int RegressionModel::rowCount(const QModelIndex&) const
{
  return m_events.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int RegressionModel::columnCount(const QModelIndex&) const
{
  return 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant RegressionModel::data(const QModelIndex &index, int role) const
{
  if((index.column() == 4 || index.column() == 5) && role == Qt::TextAlignmentRole)
    return QVariant(Qt::AlignRight);

  if(role == Qt::ForegroundRole)
  {
    bool allzero = std::all_of(m_measurementRuns.cbegin(), m_measurementRuns.cend(),
      [&](auto measurementRun)
      {return measurementRun->data(m_CoreOrUncore, index.row()).mean() == 0.0f;});

    if( m_measurementRuns.empty() || allzero)
      return QVariant(QColor(Qt::lightGray));
    return QVariant();
  }

  if(role == Qt::ToolTipRole)
  {
    QString tooltipText = "<table><tr><td>";
    for(auto measurementRun : m_measurementRuns)
    {
      tooltipText += measurementRun->command() + ":</td><td align='right'>";
      tooltipText += QString::number(measurementRun->data(m_CoreOrUncore, index.row()).mean(), 'f', 2);
      tooltipText += "</td></tr><tr><td>";
    }
    tooltipText += "</td></tr></table>";
    return QVariant(tooltipText);
  }

  if(role != Qt::DisplayRole)
    return QVariant();

  LinearRegressionResult linearRegressionResult = regressionResultForRow(index);
  switch(index.column())
  {
  case 0:
    return QVariant(m_events[index.row()].toObject()["UMask"]);
  case 1:
    return QVariant(m_events[index.row()].toObject()["EventCode"]);
  case 2:
    return QVariant(m_events[index.row()].toObject()["EventName"]);
  case 3:
    return QVariant(m_events[index.row()].toObject()["BriefDescription"]);
  case 4:
    if(m_measurementRuns.empty()) return QVariant();
    if(!linearRegressionResult.valid) return QVariant();
    return QVariant("Linear");
  case 5:
    if(m_measurementRuns.empty()) return QVariant();
    if(!linearRegressionResult.valid) return QVariant();
    return QVariant(QString::number(linearRegressionResult.a, 'f', 2) + "x + " +
                    QString::number(linearRegressionResult.b, 'f', 2) + " , " +
                    "R² = " + QString::number(linearRegressionResult.Rsquared, 'f', 2));
  }
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant RegressionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if( orientation!= Qt::Horizontal)
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  switch(section)
  {
    case 0:
      return QVariant("Mask");
    case 1:
      return QVariant("Event");
    case 2:
      return QVariant("Name");
    case 3:
      return QVariant("Description");
    case 4:
      return QVariant("RegressionType");
    case 5:
      return QVariant("RegressionDetails");
  }
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags RegressionModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

double RegressionModel::regressionError(Eigen::MatrixXf x, Eigen::VectorXf y, LinearRegressionResult regressionResult) const
{
  //https://en.wikipedia.org/wiki/Coefficient_of_determination
  float ymean = 0;
  for(int i = 0; i < y.rows(); i++)
    ymean += y(i);

  ymean /= y.rows();

  float SStot = 0;
  for(int i = 0; i < y.rows(); i++ )
    SStot += std::pow(y(i) - ymean, 2);

  float SSres = 0;
  for(int i = 0; i < y.rows(); i++)
  {
    auto functionValue = regressionResult.a * x(i, 1) + regressionResult.b;
    SSres += std::pow(y(i) - functionValue, 2);
  }

  if(SStot == 0)
    return 0.0f;

  return 1.0f - (SSres/SStot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

LinearRegressionResult RegressionModel::performRegression(Eigen::MatrixXf x, Eigen::VectorXf y) const
{
  LinearRegressionResult result;
  //Do the regression
  Eigen::MatrixXf regResult = ((x.transpose() * x).inverse() * x.transpose()) * y;
  result.a = regResult(1);
  result.b = regResult(0);
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

LinearRegressionResult RegressionModel::regressionResultForRow(QModelIndex index) const
{
  auto dataSetSize = std::accumulate(m_measurementRuns.cbegin(), m_measurementRuns.cend(), 0,
          [&](size_t acc, auto measurementRun)
          {return acc + measurementRun->data(m_CoreOrUncore,index.row()).data().size();});

  Eigen::MatrixXf x; x.resize(dataSetSize, 2);
  Eigen::VectorXf y; y.resize(dataSetSize);

  //hacky co-iterate
  int i = 0 ;
  // assuming 0, 1, 2 for x
  float x_value = 0;
  bool dataZero = true;

  for(auto measurementRun : m_measurementRuns)
  {
    for(const auto datapoint : measurementRun->data(m_CoreOrUncore,index.row()).data())
    {
      if(datapoint != 0.0f) dataZero = false;
      x(i, 0) = 1.0f;
      x(i, 1) = x_value;
      y(i) = datapoint;
      i++;
    }
    x_value++;
  }

  //If all is 0 regression will fail
  if(dataZero)
    return LinearRegressionResult({0,0,0});

  LinearRegressionResult result = performRegression(x,y);
  result.Rsquared = regressionError(x,y,result);
  if(result.Rsquared!=0.0f && result.a != 0.0f && result.b != 0.0f) result.valid = true;
  return result;
}
