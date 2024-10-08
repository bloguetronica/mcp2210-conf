QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Added to provide backwards compatibility (C++11 support)
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    QMAKE_CXXFLAGS += -std=c++11
}

TARGET = mcp2210-conf
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    common.cpp \
    configuration.cpp \
    configurationreader.cpp \
    configurationwriter.cpp \
    configuratorwindow.cpp \
    libusb-extra.c \
    main.cpp \
    mainwindow.cpp \
    mcp2210.cpp \
    mcp2210eeprom.cpp \
    passworddialog.cpp \
    statusdialog.cpp

HEADERS += \
    aboutdialog.h \
    common.h \
    configuration.h \
    configurationreader.h \
    configurationwriter.h \
    configuratorwindow.h \
    libusb-extra.h \
    mainwindow.h \
    mcp2210.h \
    mcp2210eeprom.h \
    mcp2210limits.h \
    passworddialog.h \
    statusdialog.h

FORMS += \
    aboutdialog.ui \
    configuratorwindow.ui \
    mainwindow.ui \
    passworddialog.ui \
    statusdialog.ui

TRANSLATIONS += \
    translations/mcp2210-conf_en.ts \
    translations/mcp2210-conf_en_US.ts \
    translations/mcp2210-conf_pt.ts \
    translations/mcp2210-conf_pt_PT.ts

LIBS += -lusb-1.0

RESOURCES += \
    resources.qrc

# Added installation option
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin
    icon.files += icons/mcp2210-conf.png
    icon.path = $$PREFIX/share/icons/hicolor/128x128/apps
    shortcut.files = misc/mcp2210-conf.desktop
    shortcut.path = $$PREFIX/share/applications
    INSTALLS += icon
    INSTALLS += shortcut
}

!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icons/mcp2210-conf.png \
    misc/mcp2210-conf.desktop
