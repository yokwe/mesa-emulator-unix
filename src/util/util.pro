TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Perf.h   GuiOp.h   JSONUtil.h   Setting.h   ByteBuffer.h   Network.h   NameMap.h
SOURCES += Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp ByteBuffer.cpp Network.cpp NameMap.cpp

HEADERS += Debug.h OpaqueType.h

linux  : SOURCES += Network_dummy.cpp
macx:    SOURCES += Network_dummy.cpp

freebsd {
	HEADERS += BPF.h   Network_freebsd.h
	SOURCES += BPF.cpp Network_freebsd.cpp
}


###############################################
include(../common.pro)
