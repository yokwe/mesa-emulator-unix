TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Setting.h   Perf.h   GuiOp.h
SOURCES += Util.cpp Setting.cpp Perf.cpp GuiOp.cpp

HEADERS += Debug.h


###############################################
include(../common.pro)
