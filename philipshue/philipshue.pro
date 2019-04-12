include(../plugins.pri)

TARGET = $$qtLibraryTarget(nymea_devicepluginphilipshue)

QT += network

SOURCES += \
    devicepluginphilipshue.cpp \
    #huebridgeconnection.cpp \
    #light.cpp \
    huebridge.cpp \
    huelight.cpp \
    pairinginfo.cpp \
    hueremote.cpp \
    huedevice.cpp \
    hueoutdoorsensor.cpp

HEADERS += \
    devicepluginphilipshue.h \
    #huebridgeconnection.h \
    #light.h \
    #lightinterface.h \
    huebridge.h \
    huelight.h \
    pairinginfo.h \
    hueremote.h \
    huedevice.h \
    hueoutdoorsensor.h



