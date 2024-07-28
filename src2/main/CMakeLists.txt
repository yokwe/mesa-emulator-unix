#
# man
#

add_executable (
  main
  main.cpp
  )

add_dependencies(main util mesa)

target_link_libraries (main mesa util)
