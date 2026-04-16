QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17



SOURCES += \
    celldelegate.cpp \
    commands.cpp \
    formulaengine.cpp \
    main.cpp \
    mainwindow.cpp \
    spreadsheetmodel.cpp \
    toolbarManger.cpp

HEADERS += \
    celldata.h \
    celldelegate.h \
    commands.h \
    formulaengine.h \
    mainwindow.h \
    spreadsheetmodel.h \
    toolbarManger.h

FORMS += \
    mainwindow.ui


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
