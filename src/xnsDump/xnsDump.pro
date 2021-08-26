TARGET   = xnsDump

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
linux  : SOURCES += main.cpp
macx:    SOURCES += main_bpf.cpp
freebsd: SOURCES += main_bpf.cpp

LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/xns/libxns.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/xns/libxns.a


###############################################
include(../common.pro)
