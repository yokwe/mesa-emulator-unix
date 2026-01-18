#
# floppy
#

add_executable(floppy)

target_sources(
  floppy
  PUBLIC
    ByteBuffer.h
    floppy.h
  PRIVATE
    ByteBuffer.cpp
    floppy.cpp
    main.cpp
)

add_dependencies(floppy util)

target_link_libraries (floppy util)
