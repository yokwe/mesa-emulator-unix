TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Perf.h   GuiOp.h   JSONUtil.h   Setting.h   ByteBuffer.h   Network.h
SOURCES += Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp ByteBuffer.cpp Network.cpp

HEADERS += Debug.h

linux  : SOURCES += Network_dummy.cpp
macx:    SOURCES += Network_dummy.cpp

freebsd {
	SOURCES += Network_freebsd.cpp
	
	HEADERS += BPF.h
	SOURCES += BPF.cpp
}


###############################################
include(../common.pro)
