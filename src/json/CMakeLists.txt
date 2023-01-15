#
# json
#

add_executable (
  json
  main.cpp
  json.cpp
  json_handler.cpp
  handler_basic.cpp
  handler_dot.cpp
  json_token.cpp
  )

add_dependencies(json util)

target_link_libraries (json util)
