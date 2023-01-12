#
# json
#

add_executable (
  json
  main.cpp
  json.cpp
  json_impl.cpp
  )

add_dependencies(json util)

target_link_libraries (json util)
