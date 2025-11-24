#
# bcd
#

add_library(bcd STATIC)

target_sources(
	bcd
	PUBLIC
		BCD.h
		BCDFile.h
		CTXRecord.h
		HTRecord.h
		Index.h
		MDRecord.h
		Symbols.h
		SymbolsIndex.h
		Timestamp.h
	PRIVATE
		BCD.cpp
		BCDFile.cpp
		CTXRecord.cpp
		HTRecord.cpp
		MDRecord.cpp
		Symbols.cpp
		Timestamp.cpp
)

target_link_libraries(bcd util)