#
# util
#

list(APPEND fileList Util.cpp)

add_library (util SHARED ${fileList})

target_link_libraries(util log4cxx)