#
# json
#

list(APPEND
  fileList
  json.cpp
  stream_json.cpp
  )

# libjson
add_library (json STATIC ${fileList})


# json-main
add_executable (
  json-main
  main.cpp
  )

add_dependencies(json-main util)

target_link_libraries (json-main json util)

# json-gen
add_executable (
  json-gen
  gen.cpp
)

add_dependencies(json-gen json util)

target_link_libraries(json-gen json util)
  