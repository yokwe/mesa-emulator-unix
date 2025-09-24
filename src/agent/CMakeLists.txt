#
# agent
#

add_library(agent STATIC)

target_sources(
	agent
	PUBLIC
    Agent.h
    AgentBeep.h
    AgentDisk.h
    AgentDisplay.h
    AgentFloppy.h
    AgentKeyboard.h
    AgentMouse.h
    AgentNetwork.h
    AgentProcessor.h
    AgentStream.h
    DiskFile.h
    StreamBoot.h
    StreamCopyPaste.h
    StreamPCFA.h
    StreamTCP.h
    StreamWWC.h
  PRIVATE
    Agent.cpp
    AgentBeep.cpp
    AgentDisk.cpp
    AgentDisplay.cpp
    AgentFloppy.cpp
    AgentKeyboard.cpp
    AgentMouse.cpp
    AgentNetwork.cpp
    AgentProcessor.cpp
    AgentStream.cpp
    DiskFile.cpp
    StreamBoot.cpp
    StreamCopyPaste.cpp
    StreamPCFA.cpp
    StreamTCP.cpp
    StreamWWC.cpp
)

add_dependencies(agent util)
