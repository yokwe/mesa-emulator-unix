#
# bcdFile
#

add_library(bcdFile STATIC)

target_sources(
	bcdFile
	PUBLIC
        BCD.h
        BCDFile.h
        BTIndex.h
        BodyRecord.h
        CTXIndex.h
        CTXRecord.h
        ENIndex.h
        ENRecord.h
        EXTIndex.h
        EXTRecord.h
        FTIndex.h
        FTRecord.h
        HTIndex.h
        HTRecord.h
        Index.h
        LTIndex.h
        LTRecord.h
        MDIndex.h
        MDRecord.h
        MTIndex.h
        MTRecord.h
        NameRecord.h
        SEIndex.h
        SERecord.h
        SGIndex.h
        SGRecord.h
        Symbol.h
        Tree.h
        TreeIndex.h
        Type.h
    PRIVATE
        BCD.cpp
        BCDFile.cpp
        BodyRecord.cpp
        CTXRecord.cpp
        ENRecord.cpp
        HTIndex.cpp
        HTRecord.cpp
        LTRecord.cpp
        MDRecord.cpp
        MTRecord.cpp
        SERecord.cpp
        SGRecord.cpp
        Symbol.cpp
        Tree.cpp
        Type.cpp
)

target_link_libraries(bcdFile util)
