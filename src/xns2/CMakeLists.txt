#
# xns
#

add_library(xns STATIC)

target_sources(
	xns
	PUBLIC
		Type.h
	PRIVATE
		Type.cpp
)

target_link_libraries(xns util log4cxx)