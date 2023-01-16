#
# json
#

add_executable (
  json
  main.cpp
  json.cpp
  json_token.cpp
  //
  json.h
  json_token.h
  token_basic.h
  token_dot.h
  token_record.h
  )

add_dependencies(json util)

target_link_libraries (json util)
