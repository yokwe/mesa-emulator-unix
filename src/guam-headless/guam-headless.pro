TARGET = guam-headless

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline

linux: QMAKE_POST_LINK = sudo setcap CAP_NET_RAW+pe $(TARGET)

LIBS += ../../tmp/build/opcode/libopcode.a
LIBS += ../../tmp/build/mesa/libmesa.a
LIBS += ../../tmp/build/agent/libagent.a
LIBS += ../../tmp/build/util/libutil.a
LIBS += ../../tmp/build/trace/libtrace.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/opcode/libopcode.a
POST_TARGETDEPS += ../../tmp/build/mesa/libmesa.a
POST_TARGETDEPS += ../../tmp/build/agent/libagent.a
POST_TARGETDEPS += ../../tmp/build/util/libutil.a
POST_TARGETDEPS += ../../tmp/build/trace/libtrace.a

# Input
SOURCES += main.cpp


###############################################
include(../common.pro)
