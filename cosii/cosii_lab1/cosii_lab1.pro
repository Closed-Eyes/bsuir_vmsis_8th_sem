#-------------------------------------------------
#
# Project created by QtCreator 2013-02-10T22:19:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cosii_lab1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    converter.cpp \
    analiser.cpp \
    calc.cpp

HEADERS  += mainwindow.h \
    converter.h \
    analiser.h \
    calc.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d

