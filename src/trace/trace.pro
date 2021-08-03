TARGET   = trace

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Trace.h   Module.h
SOURCES += Trace.cpp Module.cpp


###############################################
include(../common.pro)
