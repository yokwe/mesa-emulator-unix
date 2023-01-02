#
# xnsDump
#

add_executable (
  xnsDump
  main.cpp
  )

add_dependencies(xnsDump courier xns util)

target_link_libraries (xnsDump courier xns util)
