#
# util
#

add_library(util STATIC)

target_sources(
	util
	PUBLIC
		ByteBuffer.h
		Debug.h
		GuiOP.h
		OpcodeInfo.h
		Perf.h
		Setting.h
		tcl.h
    	Util.h
	PRIVATE
		ByteBuffer.cpp
		GuiOp.cpp
		OpcodeInfo.cpp
		Perf.cpp
		Setting.cpp
		tcl.cpp
		Util.cpp
)

target_link_libraries(util log4cxx)