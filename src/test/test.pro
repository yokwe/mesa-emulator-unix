TARGET   = test

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline

# Input
HEADERS += base.h
SOURCES += base.cpp

SOURCES += main.cpp memory.cpp

LIBS += ../../tmp/build/util/libutil.a

LIBS += -lcppunit -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a

###############################################
include(../common.pro)
