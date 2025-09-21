#
# tclMesa
#

add_executable(tclMesa)

target_sources(
	tclMesa
	PUBLIC
		mesa.h
		photo_image.h
	PRIVATE
		main.cpp
		MesaGuam.cpp
		MesaLog.cpp
		photo_image.cpp
)

add_dependencies     (tclMesa mesa opcode agent util)
target_link_libraries(tclMesa mesa opcode agent util tk tcl log4cxx)
