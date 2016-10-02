#ifndef SHELL_H
#define SHELL_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <QProcess>
#include <QVariantList>
#include <QMap>
#include <QTcpSocket>

namespace Memory
{

class Shell : public QObject
{
    Q_OBJECT

public:

    Shell();

    Q_PROPERTY(int pid READ pid WRITE setPid NOTIFY pidChanged)

    Q_INVOKABLE int eventsHigherThan(int memoryLatency) const;
    Q_INVOKABLE int lookupDataForBucket(int bucket);

    int pid() const;


signals:
    void pidChanged(int pid);

public slots:
    void setPid(int pid);

protected slots:
    void readTcpData();

private:
    QProcess *m_process;
    int m_pid;
    QTimer *m_measurementTimer;
    QMap<int, int> m_data;
    QTcpSocket *m_socket;

};

}
#endif // SHELL_H
