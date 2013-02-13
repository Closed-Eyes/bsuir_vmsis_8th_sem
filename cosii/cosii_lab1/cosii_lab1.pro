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
    analiser.cpp

HEADERS  += mainwindow.h \
    converter.h \
    analiser.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:\\OpenCV-2.4.3\\install\\include
LIBS += -LC:\\OpenCV-2.4.3\\install\\lib\
    -lopencv_core243d.dll \
    -lopencv_highgui243d.dll \
    -lopencv_imgproc243d.dll \
    -lopencv_features2d243d.dll \
    -lopencv_calib3d243d.dll
