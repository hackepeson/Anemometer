#-------------------------------------------------
#
# Project created by QtCreator 2017-12-13T14:31:21
#
#-------------------------------------------------

QT       += core gui  printsupport serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += static

TARGET = Anemometer_new
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#LIBS += -static-libgcc
#LIBS += -static-libstdc++
#LIBS += -lpthread
#LIBS += -static -L libwinpthread-1.dll

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp \
    dialogsettings.cpp

HEADERS += \
        mainwindow.h \
    qcustomplot.h \
    dialogsettings.h

FORMS += \
        mainwindow.ui \
    dialogsettings.ui

RESOURCES += \
    resource.qrc
