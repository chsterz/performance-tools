#include "ttestmodel.h"

#include <qdebug.h>
#include <stdexcept>
#include <cmath>

TTestModel::TTestModel(QJsonArray events,
                       MeasurementRun& measurement1,
                       MeasurementRun& measurement2,
                       EventsGroup coreOrUncore,
                       QObject *parent)
  : QAbstractTableModel(parent),
    m_events(events),
    m_coreOrUncore(coreOrUncore),
    m_measurement1(measurement1),
    m_measurement2(measurement2),
    m_upIcon("://up.png"),
    m_downIcon("://down.png"),
    m_sameIcon("://same.png")
{
  if (measurement1.data().size() != measurement2.data().size())
  {
    std::domain_error("TTest: Different sizes of data, aborting");
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int TTestModel::rowCount(const QModelIndex &) const
{
  return m_events.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int TTestModel::columnCount(const QModelIndex &) const
{
  return 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant TTestModel::data(const QModelIndex &index, int role) const
{
  auto value1 = m_measurement1.means()[std::make_pair(m_coreOrUncore, index.row())];
  auto value2 = m_measurement2.means()[std::make_pair(m_coreOrUncore, index.row())];

  if(index.column()>3 && role == Qt::TextAlignmentRole)
    return QVariant(Qt::AlignRight);

  if(role == Qt::DecorationRole)
  {
    if(index.column() == 5)
    {
      auto significanceReached = (ttestFor(std::make_pair(m_coreOrUncore, index.row())) != QVariant());
      if ((value1 < value2 && significanceReached))
        return QVariant(m_upIcon);
      else if (value1 > value2 && significanceReached)
        return QVariant(m_downIcon);
      else
        return QVariant(m_sameIcon);
    }
    return QVariant();
  }

  if(role == Qt::ToolTipRole)
  {
    return m_measurement1.data().empty() ? QVariant() : QVariant("<table><tr><td>" + QString(m_measurement1.command() + ": </td>" +
                                                                 "<td align='right'>" + QString::number(value1, 'f', 0) + "</td></tr>" +
                                                                 "<tr><td>" + QString(m_measurement2.command() + ": </td>" +
                                                                 "<td align='right'>" + QString::number(value2, 'f', 0))) + "</td></tr></table>");
  }

  if(role == Qt::ForegroundRole)
  {
    if (value1 == 0.0f && value2 == 0.0f)
      return QVariant(QColor("lightgray"));
    return QVariant();
  }

  if(role != Qt::DisplayRole)
    return QVariant();
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
    return m_measurement1.data().empty() ? QVariant()
                                         : QVariant(QString::number((value2-value1)/value1 * 100.0,'f',2)+"%");
  case 5:
    return m_measurement1.data().empty() ? QVariant()
                                         : ttestFor(std::make_pair(m_coreOrUncore, index.row()));
  }
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant TTestModel::headerData(int section, Qt::Orientation orientation, int role) const
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
      return QVariant("Difference");
    case 5:
      return QVariant("TTest accuracy");
  }
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags TTestModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant TTestModel::ttestFor(EventIdentifier event) const
{

  auto mu1 = m_measurement1.data()[event].mean();
  auto mu2 = m_measurement2.data()[event].mean();

  if(mu1 < 0.01f || mu2 < 0.01f)
    return QVariant();

  auto s1 = m_measurement1.data()[event].standardDeviation();
  auto s2 = m_measurement2.data()[event].standardDeviation();

  //explicitly casting for later calculation
  double n1 = m_measurement1.data()[event].repetitions;
  double n2 = m_measurement2.data()[event].repetitions;

  auto sTerm = std::sqrt(
             ((n1 - 1) * std::pow(s1, 2) + (n2 - 1) * std::pow(s2, 2))
             /
             (n1 + n2 - 2));

  auto rootTerm = std::sqrt( (1/n1) + (1/n2) );

  auto t = std::abs(mu1 - mu2)
           /
           (sTerm * rootTerm);

  return lookupTTestFor(t,n1+n2-2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant TTestModel::lookupTTestFor(double t, double dof) const
{
  if (dof > 50) dof = 50;
  double reachedConfidence = 0;
  for(size_t confidenceIndex = 0; confidenceIndex < s_confidences.size(); confidenceIndex++)
  {
    if(t>s_confidenceTable[dof][confidenceIndex]) reachedConfidence = s_confidences[confidenceIndex];
  }
  if(reachedConfidence < 0.95)
      return QVariant();
  return QVariant("p>"+QString::number(reachedConfidence));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
