TARGET   = main

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp


LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/mesa/libmesa.a
LIBS += ../../tmp/build/agent/libagent.a
LIBS += ../../tmp/build/opcode/libopcode.a
LIBS += ../../tmp/build/trace/libtrace.a
LIBS += ../../tmp/build/symbols/libsymbols.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/mesa/libmesa.a
POST_TARGETDEPS += ../../tmp/build/agent/libagent.a
POST_TARGETDEPS += ../../tmp/build/opcode/libopcode.a
POST_TARGETDEPS += ../../tmp/build/trace/libtrace.a
POST_TARGETDEPS += ../../tmp/build/symbols/libsymbols.a


###############################################
include(../common.pro)
