#
# opcode
#

add_library(opcode STATIC)

target_sources(
	opcode
	PUBLIC
    opcode2.h
  PRIVATE
    Opcode_bitblt.cpp
    Opcode_block.cpp
    Opcode_control.cpp
    Opcode_process.cpp
    Opcode_special.cpp
    opcode2.cpp
    OpcodeEsc.cpp
    OpcodeMop0xx.cpp
    OpcodeMop1xx.cpp
    OpcodeMop2xx.cpp
    OpcodeMop3xx.cpp
)

add_dependencies(opcode util)
