#ifndef QCORELISTMODEL_H
#define QCORELISTMODEL_H

#include <QAbstractListModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "measurement.h"

class EventListModel : public QAbstractTableModel
{
public:
  EventListModel(QJsonArray& events, std::vector<MeasurementRun>& measurementRuns, EventsGroup coreOrUncore , QObject *parent);
  int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

  void setMeasurementIndex(int measurementIndex);
  QVector<int> getSelected();

  //retrieve valid perf register Code
  QString getPerfRegisterCode(size_t index);
  QStringList getAllRegisters() const;
  QStringList getAllSelectedRegisters() const;

signals:

public slots:

private:
  QJsonArray& m_events;
  EventsGroup m_coreOrUncore;
  std::vector<MeasurementRun>& m_measurementRuns;
  size_t m_measurementIndex;
};

#endif // QCORELISTMODEL_H
