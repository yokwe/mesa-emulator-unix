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
		DiskFile.h
		MesaBuffer.h
		net.h
		Perf.h
		StringPrinter.h
		tcl.h
		trace.h
		ThreadControl.h
    	Util.h
		watchdog.h
	PRIVATE
		BPF.cpp
		ByteBuffer.cpp
		DiskFile.cpp
		MesaBuffer.cpp
		net.cpp
		Perf.cpp
		StringPrinter.cpp
		tcl.cpp
		trace.cpp
		ThreadControl.cpp
		Util.cpp
		watchdog.cpp
)

target_link_libraries(util log4cxx)