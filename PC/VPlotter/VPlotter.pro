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
    vplotterrenderer.cpp \
    graphicseffects.cpp \
    commandlistexecutor.cpp \
    convertform.cpp \
    convertimagealgorithms.cpp \
    graphics_view_zoom.cpp

HEADERS  += mainwindow.h \
    imageconverter.h \
    vplotterrenderer.h \
    graphicseffects.h \
    commandlistexecutor.h \
    convertform.h \
    convertimagealgorithms.h \
    graphics_view_zoom.h

FORMS    += mainwindow.ui \
    convertform.ui

RESOURCES += \
    assets/resources.qrc
