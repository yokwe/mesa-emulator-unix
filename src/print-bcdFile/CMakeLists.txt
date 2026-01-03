#
# print-bcdFile
#

add_executable (
  print-bcdFile
  main.cpp
  )

add_dependencies(print-bcdFile showType bcdFile util)

target_link_libraries (print-bcdFile showType bcdFile util)
