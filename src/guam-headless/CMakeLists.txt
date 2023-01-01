#
# guam-headless
#

add_executable (
  guam-headless
  main.cpp
  )

add_dependencies(guam-headless mesa opcode agent util trace)

target_link_libraries (guam-headless mesa opcode agent util trace cppunit)
