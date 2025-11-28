#
# bcd
#

add_library(bcd STATIC)

target_sources(
	bcd
	PUBLIC
		BCD.h
		BCDFile.h
		BTRecord.h
		CTXRecord.h
		EXTRecord.h
		HTRecord.h
		Index.h
		LTRecord.h
		MDRecord.h
		SERecord.h
		Symbols.h
		SymbolsIndex.h
		Timestamp.h
		Tree.h
	PRIVATE
		BCD.cpp
		BCDFile.cpp
		BTRecord.cpp
		CTXRecord.cpp
		EXTRecord.cpp
		HTRecord.cpp
		LTRecord.cpp
		MDRecord.cpp
		SERecord.cpp
		Symbols.cpp
		Timestamp.cpp
		Tree.cpp
)

target_link_libraries(bcd util)