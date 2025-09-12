#
# tclMesa
#

add_executable(tclMesa)

target_sources(
	tclMesa
	PUBLIC
		mesa.h
	PRIVATE
		main.cpp
		MesaLog.cpp
)

target_link_libraries(tclMesa util tcl log4cxx)
