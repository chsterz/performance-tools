#include "Shell.h"
#include <QFile>
#include <QThread>
#include <signal.h>

#include <limits>
#include <iostream>
#include <QNetworkAccessManager>

namespace Memory
{

Shell::Shell()
{
  m_socket = new QTcpSocket(this);
  connect( m_socket, SIGNAL(readyRead()), SLOT(readTcpData()) );
  m_socket->connectToHost("remote.host", 9999);
  qDebug() << "Waiting for Server at remote.host";
  m_socket->waitForConnected();
  qDebug() << "connected!";
}

int Shell::lookupDataForBucket(int bucket)
{
  return m_data[bucket];
}

struct measurementPackage
{
  qint32 bucket;
  qint32 value;
};

void Shell::readTcpData()
{
    QByteArray data = m_socket->readAll();
    measurementPackage *packages = reinterpret_cast<measurementPackage*>(data.data());
    for(size_t i = 0; i < data.size()/sizeof(measurementPackage); i++)
    {
      m_data[packages[i].bucket] += packages[i].value;
      qDebug() << "current " << packages[i].bucket << ": " << m_data[packages[i].bucket];
    }
}

int Shell::eventsHigherThan(int memoryLatencyThreshold) const
{
    QRegExp rx;
    rx.setPattern("(.*)?\,");
    QProcess p;
    p.setReadChannel(QProcess::StandardError);
    QString commandString = "perf stat -x, -e cpu/event=0xCD,umask=0x01,ldlat="+QString::number(memoryLatencyThreshold)
            +"/pp "
            + "--pid "+ QString::number(m_pid);
    p.start(commandString);

    QThread::msleep(20);

    kill(pid_t(p.pid()), SIGINT);
    p.waitForFinished();
    QString retString;
    retString = p.readAll();

    return retString.split(",")[0].toInt();
}

int Shell::pid() const
{
    return m_pid;
}

void Shell::setPid(int pid)
{
    if (pid != m_pid)
    {
        m_pid = pid;
        emit pidChanged(pid);
    }
}

}
