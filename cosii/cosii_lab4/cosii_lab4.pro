#-------------------------------------------------
#
# Project created by QtCreator 2013-03-03T13:00:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cosii_lab4
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    hopfieldnetwork.cpp \
    perceptron.cpp

HEADERS  += mainwindow.h \
    hopfieldnetwork.h \
    perceptron.h

FORMS    += mainwindow.ui
