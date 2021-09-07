TARGET   = xnsServer

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp

HEADERS += Listener.h   PEXListener.h   Server.h   SPPListener.h   SPPQueue.h   SPPServer.h
SOURCES += Listener.cpp PEXListener.cpp Server.cpp SPPListener.cpp SPPQueue.cpp SPPServer.cpp

HEADERS += SPPPacket.h
SOURCES += SPPPacket.cpp


LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a
LIBS += ../../tmp/build/courier/libcourier.a
LIBS += ../../tmp/build/courierImpl/libcourierImpl.a
LIBS += ../../tmp/build/xnsServerImpl/libxnsServerImpl.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a
POST_TARGETDEPS += ../../tmp/build/courier/libcourier.a
POST_TARGETDEPS += ../../tmp/build/courierImpl/libcourierImpl.a
POST_TARGETDEPS += ../../tmp/build/xnsServerImpl/libxnsServerImpl.a


###############################################
include(../common.pro)
