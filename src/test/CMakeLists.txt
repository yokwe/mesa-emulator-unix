#
# test
#

add_executable (
  test
  testBase.cpp
  testAgent.cpp testMain.cpp testMemory.cpp testOpcode_000.cpp testOpcode_100.cpp testOpcode_200.cpp
  testOpcode_300.cpp testOpcode_esc.cpp testPilot.cpp testType.cpp
  testUtil.cpp
  )

add_dependencies(test mesa opcode agent util trace)

target_link_directories(
  test
  PRIVATE tmp/build/mesa tmp/build/opcode tmp/build/agent tmp/build/util tmp/build/trace)

target_link_libraries (test mesa opcode agent util trace cppunit)
