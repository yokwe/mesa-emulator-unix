#
# xnsTimeServer
#

add_executable (
  xnsTimeServer
  main.cpp
  )

add_dependencies(xnsTimeServer xns util)

target_link_libraries (xnsTimeServer xns util)
