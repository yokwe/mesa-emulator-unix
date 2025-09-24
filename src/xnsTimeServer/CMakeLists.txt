#
# xnsTimeServer
#

add_executable (
  xnsTimeServer
  main.cpp
  )

add_dependencies(xnsTimeServer util)

target_link_libraries (xnsTimeServer util)
