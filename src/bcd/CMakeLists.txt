#
# bcd
#

add_library(bcd STATIC)

target_sources(
	bcd
	PUBLIC
		BCD.h
		BCDFile.h
		HTRecord.h
		Index.h
		Symbols.h
		Timestamp.h
	PRIVATE
		BCD.cpp
		BCDFile.cpp
		HTRecord.cpp
		Symbols.cpp
		Timestamp.cpp
)

target_link_libraries(bcd util)