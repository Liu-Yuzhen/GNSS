QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    contentwidget.cpp \
    datatype.cpp \
    date.cpp \
    dockwidget.cpp \
    dpwidget.cpp \
    filemanager.cpp \
    gnssfile.cpp \
    interpolater.cpp \
    main.cpp \
    mainwindow.cpp \
    massfuncs.cpp \
    observer.cpp \
    positionwidget.cpp

HEADERS += \
    contentwidget.h \
    datatype.h \
    date.h \
    dockwidget.h \
    dpwidget.h \
    filemanager.h \
    gnssfile.h \
    interpolater.h \
    mainwindow.h \
    massfuncs.h \
    mat.h \
    observer.h \
    positionwidget.h

FORMS += \
    contentwidget.ui \
    dockwidget.ui \
    dpwidget.ui \
    mainwindow.ui \
    positionwidget.ui

TRANSLATIONS += \
    gnss_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
