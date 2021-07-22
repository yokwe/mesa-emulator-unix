TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Setting.h   Perf.h
SOURCES += Util.cpp Setting.cpp Perf.cpp

HEADERS += Debug.h


###############################################
include(../common.pro)
