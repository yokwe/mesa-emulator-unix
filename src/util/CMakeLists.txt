#
# util
#

add_library(util STATIC)

target_sources(
	util
	PUBLIC
		BPF.h
		ByteBuffer.h
		Debug.h
		GuiOP.h
		net.h
		OpcodeInfo.h
		Perf.h
		tcl.h
		trace.h
		ThreadControl.h
    	Util.h
	PRIVATE
		BPF.cpp
		ByteBuffer.cpp
		GuiOp.cpp
		net.cpp
		OpcodeInfo.cpp
		Perf.cpp
		tcl.cpp
		trace.cpp
		ThreadControl.cpp
		Util.cpp
)

target_link_libraries(util log4cxx)