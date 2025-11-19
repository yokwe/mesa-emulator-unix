#
# bcd
#

add_library(bcd STATIC)

target_sources(
	bcd
	PUBLIC
		BCD.h
		BCDFile.h
		Index.h
		Symbols.h
	PRIVATE
		BCD.cpp
		BCDFile.cpp
		Symbols.cpp
)

target_link_libraries(bcd util)