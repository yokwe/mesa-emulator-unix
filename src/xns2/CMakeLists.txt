#
# xns
#

add_library(xns STATIC)

target_sources(
	xns
	PUBLIC
		Ethernet.h
		IDP.h
		Type.h
	PRIVATE
		Ethernet.cpp
		IDP.cpp
		Type.cpp
)

target_link_libraries(xns util log4cxx)