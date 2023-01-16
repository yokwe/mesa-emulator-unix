#
# json
#

add_executable (
  json
  main.cpp
  json.cpp
  json_token.cpp
  )

add_dependencies(json util)

target_link_libraries (json util)
