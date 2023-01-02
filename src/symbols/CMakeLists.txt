#
# symbols
#

list(APPEND
  fileList
  
  BCD.cpp BCDFile.cpp BTIndex.cpp CTXIndex.cpp ExtIndex.cpp HTIndex.cpp LTIndex.cpp MDIndex.cpp SEIndex.cpp Symbols.cpp Tree.cpp
  DumpSymbol.cpp ShowType.cpp
  )

add_library (symbols STATIC ${fileList})
