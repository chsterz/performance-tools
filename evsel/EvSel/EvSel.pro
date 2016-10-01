#-------------------------------------------------
#
# Project created by QtCreator 2015-10-05T11:54:56
#
#-------------------------------------------------

QT += core gui network
CONFIG += c++14
QMAKE_CXXFLAGS += -Wpedantic -Wall -Werror

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EvSel
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    eventlistmodel.cpp \
    measurement.cpp \
    measuermentrunmodel.cpp \
    ttestmodel.cpp \
    regressionmodel.cpp

HEADERS  += mainwindow.h \
    eventlistmodel.h \
    measurement.h \
    measuermentrunmodel.h \
    ttestmodel.h \
    regressionmodel.h

FORMS    += mainwindow.ui \
    dialog.ui

RESOURCES += \
    icons.qrc
