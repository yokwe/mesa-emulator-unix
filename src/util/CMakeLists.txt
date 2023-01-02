#
# util
#

list(APPEND fileList Util.cpp Perf.cpp GuiOp.cpp JSONUtil.cpp Setting.cpp ByteBuffer.cpp Network.cpp NameMap.cpp)

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL FreeBSD)
  list(APPEND fileList BPF.cpp Network_freebsd.cpp)
else()
  list(APPEND fileList Network_dummy.cpp)
endif()

add_library (util STATIC ${fileList})
