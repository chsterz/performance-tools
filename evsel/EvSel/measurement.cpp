#include "measurement.h"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <cmath>
#include <set>

///////////////////////////////////////////////////////////////////////////////////////////////////

void Measurement::operator<<(double value)
{
  values.push_back(value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<double>& Measurement::data() const
{
  return values;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

double Measurement::mean()
{
  if(!std::isnan(m_mean))
    return m_mean;

  m_mean = std::accumulate(values.cbegin(), values.cend(), 0.0f) / repetitions;
  return m_mean;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

double Measurement::loggedMean()
{
  if(!std::isnan(m_loggedMean))
    return m_loggedMean;

  m_loggedMean = std::accumulate(values.cbegin(), values.cend(), 0.0f,
                                 [](auto acc, auto value){return acc + std::log(value);});
  return m_loggedMean;
}

///////////////////////////////////////////////////////////////////////////////////////////////////


double Measurement::standardDeviation()
{
  if(!std::isnan(m_standardDeviation))
    return m_standardDeviation;

  auto mu = mean();
  m_standardDeviation = std::accumulate(values.cbegin(), values.cend(), 0.0f,
                                        [mu](auto acc, auto value){return acc + std::pow(value-mu, 2);});
  m_standardDeviation = std::sqrt(m_standardDeviation / (repetitions-1)); // Bessel Correction!!!
  return m_standardDeviation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

double Measurement::loggedStandardDeviation()
{
  if(!std::isnan(m_loggedStandardDeviation))
    return m_loggedStandardDeviation;

  auto lMean = loggedMean();
  m_loggedStandardDeviation = std::accumulate(values.cbegin(), values.cend(), 0.0f,
                                              [lMean](auto acc, auto value){return acc + std::pow(std::log(value)-lMean, 2);});
  m_loggedStandardDeviation = std::sqrt(m_loggedStandardDeviation / (repetitions-1)); //Bessel Correction!!!
  return m_loggedStandardDeviation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unordered_map<EventIdentifier, double> MeasurementRun::means()
{
  if(m_data.empty())
    return m_means;

  if (!m_means.empty())
    return m_means;

  for(auto &entry : m_data)
    m_means[entry.first] = entry.second.mean();

  return m_means;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const QString &MeasurementRun::command()
{
  return m_command;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unordered_map<EventIdentifier, Measurement>& MeasurementRun::data()
{
  return m_data;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

Measurement& MeasurementRun::data(EventsGroup group, int index)
{
  return m_data[std::make_pair(group, index)];
}





