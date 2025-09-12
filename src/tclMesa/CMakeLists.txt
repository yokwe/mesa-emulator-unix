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
		MesaGuam.cpp
		MesaLog.cpp
)

add_dependencies     (tclMesa mesa opcode agent util)
target_link_libraries(tclMesa mesa opcode agent util tcl log4cxx)
