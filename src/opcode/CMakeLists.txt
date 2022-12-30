#
# opcode
#

list(APPEND
  fileList
  Interpreter.cpp Opcode.cpp
  Opcode_bitblt.cpp Opcode_block.cpp Opcode_control.cpp Opcode_process.cpp Opcode_special.cpp
  OpcodeMop0xx.cpp OpcodeMop1xx.cpp OpcodeMop2xx.cpp OpcodeMop3xx.cpp
  OpcodeEsc.cpp
  )

add_library (opcode STATIC ${fileList})
