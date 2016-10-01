#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqml.h>

#include "Shell.h"

static QObject * shell_factory(QQmlEngine*, QJSEngine *) {
	return new Phase::Shell();
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	qmlRegisterSingletonType<Phase::Shell>("Phase", 1, 0, "Shell", shell_factory);

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}

