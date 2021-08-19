TARGET   = xnsServer

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp

HEADERS += XNSServer.h
SOURCES += XNSServer.cpp



LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a


###############################################
include(../common.pro)
