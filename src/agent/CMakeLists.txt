#
# agent
#

list(APPEND
  fileList
  Agent.cpp AgentBeep.cpp AgentDisk.cpp AgentDisplay.cpp AgentFloppy.cpp AgentKeyboard.cpp
  AgentMouse.cpp AgentNetwork.cpp AgentProcessor.cpp AgentStream.cpp DiskFile.cpp
  StreamBoot.cpp StreamCopyPaste.cpp StreamPCFA.cpp StreamTCP.cpp StreamWWC.cpp
  )

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
  list(APPEND fileList NetworkPacket_linux.cpp)
else()
  list(APPEND fileList NetworkPacket_dummy.cpp)
endif()

add_library (agent STATIC ${fileList})
