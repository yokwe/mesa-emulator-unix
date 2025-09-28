#
# xns
#

add_library(xns STATIC)

target_sources(
	xns
	PUBLIC
		Ethernet.h
		IDP.h
		PEX.h
		Time.h
		Type.h
	PRIVATE
		Ethernet.cpp
		IDP.cpp
		PEX.cpp
		Time.cpp
		Type.cpp
)

target_link_libraries(xns util log4cxx)