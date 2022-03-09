TARGET = dumpSymbol

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline

LIBS += ../../tmp/build/symbols/libsymbols.a
LIBS += ../../tmp/build/util/libutil.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/symbols/libsymbols.a
POST_TARGETDEPS += ../../tmp/build/util/libutil.a

# Input
SOURCES += main.cpp


###############################################
include(../common.pro)
