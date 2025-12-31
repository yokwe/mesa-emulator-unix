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
        LTIndex.h
        MDIndex.h
        MesaByteBuffer.h
        MTRecord.h
        MTIndex.h
        NameRecord.h
        SEIndex.h
        SGIndex.h
        SGRecord.h
        Symbol.h
        Symbol.h
        Timestamp.h
        TreeIndex.h
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
