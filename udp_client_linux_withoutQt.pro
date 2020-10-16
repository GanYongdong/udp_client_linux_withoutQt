TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        udpclient_ins.cpp

HEADERS += \
    udpclient_ins.h

LIBS += -lpthread
