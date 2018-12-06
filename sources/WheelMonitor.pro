# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = WheelMonitor
DESTDIR = ../build/x64/Debug
QT += core sql network gui multimedia widgets serialport multimediawidgets
CONFIG += debug
DEFINES += _WINDOWS WIN64 _WINDOWS WIN64 WIN64 QT_SERIALPORT_LIB QT_WIDGETS_LIB QT_SQL_LIB QT_NETWORK_LIB QAPPLICATION_CLASS=QApplication QT_WIDGETS_LIB QT_SQL_LIB QT_NETWORK_LIB QT_SERIALPORT_LIB QT_WIDGETS_LIB QT_SQL_LIB QT_NETWORK_LIB QT_SERIALPORT_LIB QT_MULTIMEDIA_LIB QT_MULTIMEDIAWIDGETS_LIB QT_MESSAGELOGCONTEXT
INCLUDEPATH += . \
    . \
    ./GeneratedFiles \
    . \
    ./GeneratedFiles/Release \
    ./../singleapplication \
    ./GeneratedFiles/release \
    ./GeneratedFiles \
    $(QTDIR)/mkspecs/win32-msvc \
    ./GeneratedFiles \
    ./GeneratedFiles/debug \
    ./GeneratedFiles \
    $(QTDIR)/mkspecs/win32-msvc \
    ./GeneratedFiles
LIBS += -lshell32 \
    -lAdvapi32
PRECOMPILED_HEADER = stdafx.h
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/$(ConfigurationName)
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
include(WheelMonitor.pri)
win32:RC_FILE = WheelMonitor.rc
