TARGET   = main

TEMPLATE = app

# run from comand line - no gui in osx
CONFIG += cmdline


# Input
SOURCES += main.cpp


LIBS += ../../tmp/build/util/libutil.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a


###############################################
include(../common.pro)
