#
# bcdFile
#

add_library(bcdFile STATIC)

target_sources(
	bcdFile
	PUBLIC
		BCD.h
		BCDFile.h
        CTXIndex.h
        ENIndex.h
        ENRecord.h
        FTIndex.h
        FTRecord.h
        HTIndex.h
        HTRecord.h
        MesaByteBuffer.h
        MTRecord.h
        MTIndex.h
        NameRecord.h
        SGIndex.h
        SGRecord.h
        Symbol.h
        Symbol.h
        Timestamp.h
    PRIVATE
        BCD.cpp
        BCDFile.cpp
        ENRecord.cpp
        HTRecord.cpp
        MesaByteBuffer.cpp
        MTRecord.cpp
        SGRecord.cpp
        Symbol.cpp
        Timestamp.cpp
)

target_link_libraries(bcdFile util)
