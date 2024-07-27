#
# util
#

list(APPEND fileList Util.cpp)

add_library (util STATIC ${fileList})

target_link_libraries(util log4cxx)