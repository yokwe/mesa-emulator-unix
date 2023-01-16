#
# json
#

add_executable (
  json
  main.cpp
  json.cpp
  handler.h
  )

add_dependencies(json util)

target_link_libraries (json util)
