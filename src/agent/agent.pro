TARGET   = agent

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Agent.h   AgentBeep.h   AgentDisk.h   AgentDisplay.h   AgentFloppy.h   AgentKeyboard.h
SOURCES += Agent.cpp AgentBeep.cpp AgentDisk.cpp AgentDisplay.cpp AgentFloppy.cpp AgentKeyboard.cpp

HEADERS += AgentMouse.h   AgentNetwork.h   AgentProcessor.h   AgentStream.h   DiskFile.h   NetworkPacket.h
SOURCES += AgentMouse.cpp AgentNetwork.cpp AgentProcessor.cpp AgentStream.cpp DiskFile.cpp

HEADRES += StreamBoot.h   StreamCopyPaste.h   StreamPCFA.h   StreamTCP.h   StreamWWC.h
SOURCES += StreamBoot.cpp StreamCopyPaste.cpp StreamPCFA.cpp StreamTCP.cpp StreamWWC.cpp

linux  : SOURCES += NetworkPacket.cpp
freebsd: SOURCES += NetworkPacket_dummy.cpp
macx:    SOURCES += NetworkPacket_dummy.cpp


###############################################
include(../common.pro)
