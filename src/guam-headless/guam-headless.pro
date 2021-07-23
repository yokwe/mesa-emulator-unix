TARGET = guam-headless

TEMPLATE = app

linux {
	QMAKE_POST_LINK = sudo setcap CAP_NET_RAW+pe $(TARGET)
}


LIBS += ../../tmp/build/opcode/libopcode.a
LIBS += ../../tmp/build/mesa/libmesa.a
LIBS += ../../tmp/build/agent/libagent.a
LIBS += ../../tmp/build/util/libutil.a

LIBS += -llog4cpp
LIBS += -lexecinfo

POST_TARGETDEPS += ../../tmp/build/opcode/libopcode.a
POST_TARGETDEPS += ../../tmp/build/mesa/libmesa.a
POST_TARGETDEPS += ../../tmp/build/agent/libagent.a
POST_TARGETDEPS += ../../tmp/build/util/libutil.a

# Input
SOURCES += main.cpp


###############################################
include(../common.pro)
