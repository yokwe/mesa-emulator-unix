#
# util
#

add_library(util SHARED)

target_sources(
	util
	PUBLIC
		Util.h
	PRIVATE
		Util.cpp
)

target_link_libraries(util log4cxx)