#-------------------------------------------------
#
# Project created by QtCreator 2012-08-03T22:25:55
#
#-------------------------------------------------

QT       += core
QT       -= gui
QT       += network

TARGET = SerialWebSocket
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    qtwebsocket/QtWebSocket/QWsSocket.cpp \
    qtwebsocket/QtWebSocket/QWsServer.cpp \
    qt-json/json.cpp \
    serialserver.cpp

HEADERS += \
    qtwebsocket/QtWebSocket/QWsSocket.h \
    qtwebsocket/QtWebSocket/QWsServer.h \
    qt-json/json.h \
    serialserver.h


#Use some QextSerialport
include(qextserialport/src/qextserialport.pri)

