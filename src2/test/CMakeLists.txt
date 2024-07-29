#
# test
#

add_executable(test)

target_sources(
	test
	PRIVATE
		testBase.cpp
		testBase.h
		testMain.cpp
		testOpcode_000.cpp
		testPilot.cpp
		testType.cpp
)

add_dependencies(test util mesa opcode)

target_link_libraries (test util mesa opcode cppunit)
