#
# mesa
#

list(APPEND fileList Dummy.cpp Memory.cpp)

add_library (mesa SHARED ${fileList})

add_dependencies(mesa util)

target_link_libraries (mesa util)
