#include "measuermentrunmodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

int MeasuermentRunModel::rowCount(const QModelIndex &) const
{
  return m_measurements.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant MeasuermentRunModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if ((uint)index.row() >= m_measurements.size())
    return QVariant();

  if(role != Qt::DisplayRole)
    return QVariant();

  return QVariant(m_measurements[index.row()].command());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QVariant MeasuermentRunModel::headerData(int, Qt::Orientation, int ) const
{
  return QVariant("Eier");
}

void MeasuermentRunModel::dataChanged()
{
  beginInsertRows(QModelIndex(),m_measurements.size(),m_measurements.size());
  endInsertRows();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
