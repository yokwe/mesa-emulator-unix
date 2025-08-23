#
# test
#

add_executable(test)

target_sources(
	test
	PUBLIC
    testBase.h
  PRIVATE
    testAgent.cpp
    testBase.cpp
    testMain.cpp
    testMemory.cpp
    testOpcode_000.cpp
    testOpcode_100.cpp
    testOpcode_200.cpp
    testOpcode_300.cpp
    testOpcode_esc.cpp
    testPilot.cpp
    testType.cpp
    testUtil.cpp
)

add_dependencies(test agent opcode mesa util)

target_link_libraries (test agent opcode mesa util cppunit)
