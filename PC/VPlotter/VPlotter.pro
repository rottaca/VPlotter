#-------------------------------------------------
#
# Project created by QtCreator 2016-11-08T12:06:37
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VPlotter
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageconverter.cpp \
    vplotterrenderer.cpp

HEADERS  += mainwindow.h \
    imageconverter.h \
    vplotterrenderer.h

FORMS    += mainwindow.ui
