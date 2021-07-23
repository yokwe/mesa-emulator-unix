TARGET   = test

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline

# Input
HEADERS += testBase.h
SOURCES += testBase.cpp

SOURCES += testAgent.cpp testMain.cpp testMemory.cpp testOpcode_000.cpp testOpcode_100.cpp testOpcode_200.cpp
SOURCES += testOpcode_300.cpp testOpcode_esc.cpp testPilot.cpp testType.cpp

LIBS += ../../tmp/build/mesa/libmesa.a
LIBS += ../../tmp/build/opcode/libopcode.a
LIBS += ../../tmp/build/agent/libagent.a
LIBS += ../../tmp/build/util/libutil.a

LIBS += -lcppunit

POST_TARGETDEPS += ../../tmp/build/mesa/libmesa.a
POST_TARGETDEPS += ../../tmp/build/opcode/libopcode.a
POST_TARGETDEPS += ../../tmp/build/agent/libagent.a
POST_TARGETDEPS += ../../tmp/build/util/libutil.a

###############################################
include(../common.pro)
