#
# showType
#

add_library(showType STATIC)

target_sources(
  showType
	PUBLIC
    ShowType.h
    SymbolOps.h
  PRIVATE
    ShowType.cpp
    SymbolOps.cpp
)

target_link_libraries(showType bcdFile util)
