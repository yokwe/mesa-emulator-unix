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
		MesaBoot.cpp
		MesaConfig.cpp
		MesaDisplay.cpp
		MesaEvent.cpp
		MesaLinkVar.cpp
		MesaLog.cpp
		MesaMemory.cpp
		MesaPerf.cpp
		MesaVariable.cpp
		photo_image.cpp
)

add_dependencies     (tclMesa mesa opcode agent util)
target_link_libraries(tclMesa mesa opcode agent util tk tcl log4cxx)
