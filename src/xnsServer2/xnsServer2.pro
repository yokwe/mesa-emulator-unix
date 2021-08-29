TARGET   = xnsServer2

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp

HEADERS += Server.h   Listener.h
SOURCES += Server.cpp Listener.cpp


LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a
LIBS += ../../tmp/build/courier/libcourier.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a
POST_TARGETDEPS += ../../tmp/build/courier/libcourier.a


###############################################
include(../common.pro)
