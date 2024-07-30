#
# opcode
#

add_library(opcode SHARED)

target_sources(
	opcode
	PUBLIC
		Interpreter.h
		Opcode.h
	PRIVATE
		Interpreter.cpp
		Opcode.cpp
		OpcodeMop0xx.cpp
		OpcodeMop1xx.cpp
)

add_dependencies(opcode mesa util)

target_link_libraries (opcode mesa util)
