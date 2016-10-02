#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqml.h>

#include "Shell.h"

static QObject * shell_factory(QQmlEngine*, QJSEngine *) {
    return new Memory::Shell();
}


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qmlRegisterSingletonType<Memory::Shell>("Memory", 1, 0, "Shell", shell_factory);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
