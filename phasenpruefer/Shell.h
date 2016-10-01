#ifndef SHELL_H
#define SHELL_H

#include <QQuickItem>
#include <QString>
#include <QDebug>
#include <QProcess>
#include <QVariantList>

namespace Phase
{

class Shell : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged)
	Q_PROPERTY(int pid READ pid WRITE setPid NOTIFY namePid)
	Q_PROPERTY(int memoryUsage READ memoryUsage WRITE setMemoryUsage NOTIFY memoryUsageChanged)


public:

	Shell();

	Q_INVOKABLE bool checkCommandExists();
	Q_INVOKABLE void startMeasurement();
	Q_INVOKABLE void stopMeasurement();
	Q_INVOKABLE int split(QVariantList points);
	Q_INVOKABLE QPoint cacheMissesSplitAt(qreal time);

	QString command() const;
	int pid() const;
	int memoryUsage() const;

signals:
	void commandChanged(QString command);
	void namePid(int pid);
	void memoryUsageChanged(int memoryUsage);

public slots:
	void setCommand(QString command);
	void setPid(int pid);
	void setMemoryUsage(int memoryUsage);

private:
	QProcess *m_process;
	QProcess *m_perf_record_process;
	QString m_command;
	int m_pid;
	int m_memoryUsage;
};

}
#endif // SHELL_H
