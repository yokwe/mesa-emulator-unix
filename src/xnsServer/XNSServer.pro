TARGET   = xnsServer

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp

HEADERS += CHServiceImpl.h   EchoServiceImpl.h   RIPServiceImpl.h   TimeServiceImpl.h   Server.h
SOURCES += CHServiceImpl.cpp EchoServiceImpl.cpp RIPServiceImpl.cpp TimeServiceImpl.cpp Server.cpp


LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a
LIBS += ../../tmp/build/courier/libcourier.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a
POST_TARGETDEPS += ../../tmp/build/courier/libcourier.a


###############################################
include(../common.pro)
