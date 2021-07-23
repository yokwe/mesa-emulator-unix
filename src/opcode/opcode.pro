TARGET   = opcode

TEMPLATE = lib
CONFIG  += staticlib

# Input

HEADERS += Interpreter.h   Opcode.h
SOURCES += Interpreter.cpp Opcode.cpp

SOURCES += Opcode_bitblt.cpp Opcode_block.cpp Opcode_control.cpp Opcode_process.cpp Opcode_special.cpp
SOURCES += OpcodeMop0xx.cpp OpcodeMop1xx.cpp OpcodeMop2xx.cpp OpcodeMop3xx.cpp
SOURCES += OpcodeEsc.cpp


###############################################
include(../common.pro)
