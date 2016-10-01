#ifndef REGRESSIONMODEL_H
#define REGRESSIONMODEL_H

#include <functional>
#include <vector>

#include <QAbstractTableModel>
#include <QJsonArray>
#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/Dense>

#include "measurement.h"

struct LinearRegressionResult
{
  float a;
  float b;
  float Rsquared;
  bool valid = false;
};

class RegressionModel : public QAbstractTableModel
{
Q_OBJECT
public:
  RegressionModel(QJsonArray& events,
                  std::vector<MeasurementRun*> measurementRuns,
                  EventsGroup coreOrUncore,
                  QObject* parent);
  int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex&) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &) const Q_DECL_OVERRIDE;

private:
  QJsonArray& m_events;
  EventsGroup m_CoreOrUncore;
  std::vector<MeasurementRun*> m_measurementRuns;

  double regressionError(Eigen::MatrixXf x, Eigen::VectorXf y, LinearRegressionResult result) const;
  LinearRegressionResult performRegression(Eigen::MatrixXf x, Eigen::VectorXf y) const;
  LinearRegressionResult regressionResultForRow(QModelIndex index) const;

};

#endif // REGRESSIONMODEL_H
