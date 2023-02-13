#
# json
#

add_executable (
  json
  main.cpp
  json.h
  json.cpp
   # stream
  stream.h
  stream_json.cpp
  stream_json.h
  )

add_dependencies(json util)

target_link_libraries (json util)
