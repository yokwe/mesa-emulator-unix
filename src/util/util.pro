TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Perf.h   GuiOp.h   JSONUtil.h   Setting.h   ModuleInfo.h
SOURCES += Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp ModuleInfo.cpp

HEADERS += Debug.h


###############################################
include(../common.pro)
