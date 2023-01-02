#
# dumpSymbol
#

add_executable (
  dumpSymbol
  main.cpp
  )

add_dependencies(dumpSymbol symbols util)

target_link_libraries (dumpSymbol symbols util)
