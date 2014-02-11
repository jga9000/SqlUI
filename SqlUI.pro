#-------------------------------------------------
#
# Project created by QtCreator 2013-06-17T23:40:20
#
#-------------------------------------------------

QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SqlUI
TEMPLATE = app

CONFIG( debug, debug|release ) {
    win32:LIBS += -Lbin -ltinysqliteapiclient
} else {
     win32:LIBS += -Lbin -ltinysqliteapiclient
}

unix:LIBS += -L/usr/local/lib/shared-ltinysqliteapiclient

SOURCES += main.cpp \
    buttonheaderview.cpp \
    mainwindow.cpp \
    headereditor.cpp

HEADERS += \
    buttonheaderview.h \
    mainwindow.h \
    headereditor.h

FORMS    += mainwindow.ui

RESOURCES     = resources.qrc
