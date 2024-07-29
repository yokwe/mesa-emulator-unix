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
		testPilot.cpp
		testType.cpp
)

add_dependencies(test util mesa)

target_link_libraries (test util mesa cppunit)
