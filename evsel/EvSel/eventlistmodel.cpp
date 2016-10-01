#include "eventlistmodel.h"

#include <QDebug>
#include <QString>
#include <QLocale>
#include <QColor>

///////////////////////////////////////////////////////////////////////////////////////////////////

EventListModel::EventListModel(QJsonArray& events,
                               std::vector<MeasurementRun>& measurementRuns,
                               EventsGroup coreOrUncore,
                               QObject *parent = nullptr)
  :QAbstractTableModel(parent),
  m_events(events),
  m_coreOrUncore(coreOrUncore),
  m_measurementRuns(measurementRuns),
  m_measurementIndex(0)
{}

///////////////////////////////////////////////////////////////////////////////////////////////////

int EventListModel::rowCount(const QModelIndex&) const
{
  return m_events.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int EventListModel::columnCount(const QModelIndex&) const
{
  return 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant EventListModel::data(const QModelIndex &index, int role) const
{
  if(index.column()==4 && role == Qt::TextAlignmentRole)
    return QVariant(Qt::AlignRight);

  if(role == Qt::ForegroundRole)
  {
    if(    m_measurementRuns.empty() || m_measurementRuns[m_measurementIndex].means().empty()
        || m_measurementRuns[m_measurementIndex].means()[std::make_pair(m_coreOrUncore, index.row())] == 0.0f )
      return QVariant(QColor(Qt::lightGray));
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
    return QVariant( m_measurementRuns.empty() ? QVariant()
                     : QVariant(QString::number(m_measurementRuns[m_measurementIndex].means()[std::make_pair(m_coreOrUncore, index.row())],'f',2)));
  default:
    return QVariant();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant EventListModel::headerData(int section, Qt::Orientation orientation, int role) const
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
      return QVariant("Sample Average");
    case 5:
      return QVariant("Correlation");
  }
  return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags EventListModel::flags(const QModelIndex&) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void EventListModel::setMeasurementIndex(int measurementIndex)
{
  beginResetModel();
  m_measurementIndex = measurementIndex;
  endResetModel();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QString EventListModel::getPerfRegisterCode(size_t index)
{
  QString uMask = m_events[index].toObject()["UMask"].toString().right(2);
  QString eventCode = m_events[index].toObject()["EventCode"].toString().right(2);
  return "r" + uMask + eventCode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QStringList EventListModel::getAllRegisters() const
{
  QStringList eventRegisters = *new QStringList();
  for(int i = 0; i < m_events.size(); i++)
  {
    QString uMask = m_events[i].toObject()["UMask"].toString().right(2);
    QString eventCode = m_events[i].toObject()["EventCode"].toString().right(2);
    eventRegisters << "r" + uMask + eventCode;
  }
  return eventRegisters;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

