#
# bcdFile
#

add_library(bcdFile STATIC)

target_sources(
	bcdFile
	PUBLIC
		BCD.h
		BCDFile.h
        FTIndex.h
        FTRecord.h
        NameRecord.h
        SGIndex.h
        SGRecord.h
        Symbol.h
        Timestamp.h
    PRIVATE
        BCD.cpp
        BCDFile.cpp
        SGIndex.cpp
        SGRecord.cpp
        Timestamp.cpp
)

target_link_libraries(bcdFile util)
