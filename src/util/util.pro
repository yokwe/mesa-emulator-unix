TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Perf.h   GuiOp.h   JSONUtil.h   Setting.h
SOURCES += Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp

HEADERS += Debug.h


###############################################
include(../common.pro)
