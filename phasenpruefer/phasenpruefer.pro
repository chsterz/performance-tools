TEMPLATE = app
CONFIG += c++11

QT += qml quick widgets charts

SOURCES += main.cpp \
    Shell.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    Shell.h
