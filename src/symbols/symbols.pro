TARGET   = symbols

TEMPLATE = lib
CONFIG  += staticlib
CONFIG  += warn_off

# Input
HEADERS += BCD.h   BCDFile.h   BTIndex.h   CTXIndex.h   ExtIndex.h   HTIndex.h   LTIndex.h   MDIndex.h   SEIndex.h   Symbols.h   Tree.h
SOURCES += BCD.cpp BCDFile.cpp BTIndex.cpp CTXIndex.cpp ExtIndex.cpp HTIndex.cpp LTIndex.cpp MDIndex.cpp SEIndex.cpp Symbols.cpp Tree.cpp

HEADERS += DumpSymbol.h   ShowType.h
SOURCES += DumpSymbol.cpp ShowType.cpp


###############################################
include(../common.pro)
