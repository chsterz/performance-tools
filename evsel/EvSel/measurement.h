#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <QString>
#include <limits>

///////////////////////////////////////////////////////////////////////////////////////////////////

enum class EventsGroup{CORE, UNCORE};
using EventIdentifier = std::pair<EventsGroup, int>;

//make EventIdentifier hashable
namespace std
{
  template<>
  struct hash<EventIdentifier>
  {
    hash(){}
    size_t operator()(const EventIdentifier &e) const
    {
      return hash<int>()(e.second) ^ (e.first == EventsGroup::CORE ? 0 : 1);
    }
  };
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class Measurement
{

public:
  Measurement()
    :
    repetitions(0),
    m_mean(std::numeric_limits<double>::quiet_NaN()),
    m_loggedMean(std::numeric_limits<double>::quiet_NaN()),
    m_standardDeviation(std::numeric_limits<double>::quiet_NaN()),
    m_loggedStandardDeviation(std::numeric_limits<double>::quiet_NaN())
  {}

  void operator<<(double value);

  const std::vector<double> &data() const;
  double mean();
  double loggedMean();
  double standardDeviation();
  double loggedStandardDeviation();
  size_t repetitions;

private:
  std::vector<double> values;

  double m_mean;
  double m_loggedMean;
  double m_standardDeviation;
  double m_loggedStandardDeviation;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

struct TTestResult
{
  float t;
  float reachedSignificance;
  bool meansDifferent;
};
using TTestResults = std::vector<TTestResult>;

///////////////////////////////////////////////////////////////////////////////////////////////////

class MeasurementRun
{
public:
  MeasurementRun(QString command)
     :m_command(command),
      m_data(),
      m_means()
  {}

public:
  std::unordered_map<EventIdentifier, Measurement>& data();
  Measurement& data(EventsGroup group, int index);
  std::unordered_map<EventIdentifier, double> means();

  TTestResult performTTest(const MeasurementRun &other);
  const QString& command();

private:
  QString m_command;
  std::unordered_map<EventIdentifier, Measurement> m_data;
  std::unordered_map<EventIdentifier, double> m_means;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
