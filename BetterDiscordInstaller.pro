#-------------------------------------------------
#
# Project created by QtCreator 2018-01-07T18:02:12
#
#-------------------------------------------------

QT       += core gui network concurrent


macx:ICON += $${_PRO_FILE_PWD_}/resources/icon.icns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BetterDiscordInstaller
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QUAZIP_STATIC

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        betterdiscordinstaller.cpp

HEADERS += \
        betterdiscordinstaller.h

FORMS += \
        betterdiscordinstaller.ui

QMAKE_CXXFLAGS += -std=c++1z


INCLUDEPATH += includes/nlohmann_json/include
INCLUDEPATH += includes/quazip/include

macx {
    LIBS += -L$${_PRO_FILE_PWD_}/includes/quazip/lib
}

unix:!macx{
    LIBS += -L$${_PRO_FILE_PWD_}/includes/quazip/lib/linux
}

LIBS += -lquazip
LIBS += -lz
