TARGET   = xnsServer2

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp

HEADERS += Server.h   Listener.h
SOURCES += Server.cpp Listener.cpp

HEADERS += EchoListener.h   RIPListener.h   PEXListener.h   TimeListener.h   CHSListener.h   SPPListener.h   CourierListener.h
SOURCES += EchoListener.cpp RIPListener.cpp PEXListener.cpp TimeListener.cpp CHSListener.cpp SPPListener.cpp CourierListener.cpp

HEADERS += CHService.h
SOURCES += CHService.cpp


LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a
LIBS += ../../tmp/build/courier/libcourier.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a
POST_TARGETDEPS += ../../tmp/build/courier/libcourier.a


###############################################
include(../common.pro)
