#
# tclMesa
#

add_executable(tclMesa)

target_sources(
	tclMesa
	PUBLIC
		keymap.h
		photo_image.h
		tclMesa.h
	PRIVATE
		keymap.cpp
		main.cpp
		MesaEvent.cpp
		MesaGuam.cpp
		MesaGuam_boot.cpp
		MesaGuam_config.cpp
		MesaGuam_display.cpp
		MesaLog.cpp
		photo_image.cpp
)

add_dependencies     (tclMesa mesa opcode agent util)
target_link_libraries(tclMesa mesa opcode agent util tk tcl log4cxx)
