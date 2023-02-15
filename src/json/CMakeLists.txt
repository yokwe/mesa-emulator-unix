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
  stream_util.h
  stream_json.h
  stream_json.cpp
  )

add_dependencies(json util)

target_link_libraries (json util)
