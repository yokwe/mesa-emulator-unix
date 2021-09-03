TARGET   = xnsServerImpl

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += CHSListener.h   EchoListener.h   RIPListener.h   SPPCourier.h   TimeListener.h
SOURCES += CHSListener.cpp EchoListener.cpp RIPListener.cpp SPPCourier.cpp TimeListener.cpp

HEADERS += PilotStream.h


###############################################
include(../common.pro)
