#pragma once

#include <QAbstractListModel>
#include <vector>
#include "measurement.h"
#include <QDebug>

class MeasuermentRunModel : public QAbstractListModel
{
Q_OBJECT
public:
  MeasuermentRunModel(std::vector<MeasurementRun>& measurements,QObject *parent=nullptr)
    : QAbstractListModel(parent),
      m_measurements(measurements)
  {}
  int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
  void dataChanged();
private:
  std::vector<MeasurementRun>& m_measurements;
};
