#
# main
#

add_executable (
  main
  main.cpp
  )

add_dependencies(main util)

target_link_libraries (main util)
