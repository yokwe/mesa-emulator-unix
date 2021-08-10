TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Perf.h   GuiOp.h   JSONUtil.h   Setting.h   ByteBuffer.h
SOURCES += Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp ByteBuffer.cpp

HEADERS += Debug.h

HEADERS += Network.h
linux  : SOURCES += Network_dummy.cpp
freebsd: SOURCES += Network_freebsd.cpp
macx:    SOURCES += Network_dummy.cpp


###############################################
include(../common.pro)
