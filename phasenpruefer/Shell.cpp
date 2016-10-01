#include "Shell.h"
#include <eigen3/Eigen/Dense>
#include <QFile>

#include <limits>
#include <iostream>

namespace Phase
{

Shell::Shell()
{

}

bool Shell::checkCommandExists()
{
	QProcess p;
	p.start("which "+m_command);
	p.waitForFinished();
	bool exists = (p.readAll()=="") ? false : true;

	return exists;
}

void Shell::startMeasurement()
{
	qDebug() << "[*] Starting Measurement on: " + m_command;
	m_process = new QProcess(this);
	m_process->start(m_command);
	m_pid = m_process->pid();
	m_perf_record_process = new QProcess(this);
	qDebug() << "perf record -e cache-misses  -o /tmp/perf.data --pid " + QString::number(m_pid);
	m_perf_record_process->start("perf record -e cache-misses  -o /tmp/perf.data --pid " + QString::number(m_pid));
}

void Shell::stopMeasurement()
{
	qDebug() << "[*] Stopping Measurement on: " + m_command;
	m_perf_record_process->terminate();
	m_perf_record_process->waitForFinished();
	m_process->terminate();
}

typedef struct{
	Eigen::MatrixXf x;
	Eigen::VectorXf y;
}Regression;

int Shell::split(QVariantList points)
{
	QPointF point;

	//omit last value, because it is always zero
	int dataSize = points.size()-1;

	Eigen::MatrixXf x; x.resize(dataSize,2);
	Eigen::VectorXf y(dataSize);

  //create the linear eq system in the form of y = beta1*x + beta2*1
	for( int i = 0; i < dataSize; i++)
	{
		point = points[i].toPointF();
		x(i, 0) = 1.0f; //beta for y-intercept
		x(i, 1) = point.x(); //beta for slope (dependent on x)

		y(i) = point.y(); 
	}

	//Error function (least squares of ax+b)
	auto error = [](Regression reg, int b, int a)->float
	{
		float result = 0;
		for(int i=0 ; i< reg.y.size(); i++)
		{
			float functionValue = a*reg.x(i, 1)+ b;
			float squarederror = std::pow(reg.y(i) - functionValue, 2);
			result+=squarederror;
		}
		return result;

	};

	//Perform all pairs of regressions
	float lowestError = std::numeric_limits<float>::max();
	float r1a, r1b;
	float r2a, r2b;
	int splitIndex = 0;

	for( int i = 2; i < dataSize; i++)
	{
		Regression reg1; reg1.x = x.topRows(i); reg1.y = y.head(i);
		Regression reg2; reg2.x = x.bottomRows(dataSize-i); reg2.y = y.tail(dataSize-i);

		Eigen::MatrixXf reg1Result = ((reg1.x.transpose() * reg1.x).inverse() * reg1.x.transpose()) * reg1.y;
		Eigen::MatrixXf reg2Result = ((reg2.x.transpose() * reg2.x).inverse() * reg2.x.transpose()) * reg2.y;

		float currentError = error(reg1,reg1Result(0),reg1Result(1)) + error(reg2,reg2Result(0),reg2Result(1));
		if (currentError < lowestError)
		{
			r1a = reg1Result(1); r1b = reg1Result(0);
			r2a = reg2Result(1); r2b = reg2Result(0);
			lowestError = currentError;
			splitIndex = i;
		}

	}
	std::cout << "r1:" << r1a << "x + " << r1b << std::endl;
	std::cout << "r2:" << r2a << "x + " << r2b << std::endl;
	std::cout << "(smallest error:" << lowestError << ")" << std::endl;

	return splitIndex;
}

QPoint Shell::cacheMissesSplitAt(qreal time)
{
	QProcess *perf_script_awk = new QProcess();
	perf_script_awk->start("perf script -i /tmp/perf.data -F time");
	perf_script_awk->waitForFinished();
	perf_script_awk->setReadChannel(QProcess::StandardOutput);
	QString before_after = perf_script_awk->readAll();
	QStringList before_after_list = before_after.split(": \n");

	float firstTimeStamp = before_after_list[0].toFloat();
	int before = 0;
	int after = 0;

	for (auto timestring : before_after_list)
	{
		if((timestring.toFloat() - firstTimeStamp) < time) before++;
		else after++;
	}
	qDebug() << before << after;
	return QPoint(before, after);
}

QString Shell::command() const
{
	return m_command;
}

int Shell::pid() const
{
	return m_pid;
}

int Shell::memoryUsage() const
{
	QRegExp rx;
	rx.setPattern("VmRSS:\\t\\s+(\\d+)");
	QProcess p;
	p.start("cat /proc/"+ QString::number(m_pid) +"/status");
	p.waitForFinished();
	QString kBString;
	if (rx.indexIn(p.readAll()) != -1) {
		kBString = rx.cap(1);
	}
	return kBString.toInt();
}

void Shell::setCommand(QString command)
{
	if (m_command == command)
		return;

	m_command = command;
	emit commandChanged(command);
}

void Shell::setPid(int pid)
{
	if (m_pid == pid)
		return;

	m_pid = pid;
	emit namePid(pid);
}

void Shell::setMemoryUsage(int memoryUsage)
{
	if (m_memoryUsage == memoryUsage)
		return;

	m_memoryUsage = memoryUsage;
	emit memoryUsageChanged(memoryUsage);
}

}
