#
# print-symbol
#

add_executable (
  print-symbol
  main.cpp
  )

add_dependencies(print-symbol bcd util)

target_link_libraries (print-symbol bcd util)
