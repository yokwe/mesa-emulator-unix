#
# floppy
#

add_executable(floppy)

target_sources(
  floppy
  PUBLIC
    floppy.h
  PRIVATE
    floppy.cpp
    main.cpp
)

add_dependencies(floppy agent util)

target_link_libraries (floppy agent util)
