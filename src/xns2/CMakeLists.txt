#
# xns
#

add_library(xns STATIC)

target_sources(
	xns
	PUBLIC
		Echo.h
		Error.h
		Ethernet.h
		IDP.h
		PEX.h
		RIP.h
		SPP.h
		Time.h
		Type.h
	PRIVATE
		Echo.cpp
		Error.cpp
		Ethernet.cpp
		IDP.cpp
		PEX.cpp
		RIP.cpp
		SPP.cpp
		Time.cpp
		Type.cpp
)

target_link_libraries(xns util log4cxx)