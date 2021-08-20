TARGET   = xns

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += XNS.h   Config.h   Courier.h
SOURCES += XNS.cpp Config.cpp Courier.cpp

HEADERS += RIP.h   Error.h   Echo.h   SPP.h   PEX.h
SOURCES += RIP.cpp Error.cpp Echo.cpp SPP.cpp PEX.cpp

HEADERS += Time.h
SOURCES += Time.cpp

HEADERS += Server.h
SOURCES += Server.cpp


###############################################
include(../common.pro)
