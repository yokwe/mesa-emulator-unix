#
# test
#

add_executable (
  test
  testBase.cpp testMain.cpp
  testPilot.cpp testType.cpp
  )

add_dependencies(test util mesa)

target_link_libraries (test util mesa cppunit)
