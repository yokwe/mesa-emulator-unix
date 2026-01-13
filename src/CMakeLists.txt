cmake_minimum_required (VERSION 3.24.0)

project(mesa-emulator-unix VERSION 2.0.0 LANGUAGES CXX)

#
# common setting
#
# host name and system
cmake_host_system_information(RESULT HOST_NAME QUERY HOSTNAME)
set(HOST_SYSTEM ${CMAKE_HOST_SYSTEM_NAME})
message(STATUS "NAME       ${HOST_NAME}")
message(STATUS "SYSTEM     ${HOST_SYSTEM}")

# output build/compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# default c++ language level
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# default compiler optimization setting
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# default compiler options
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -g -O2 -fPIC")

# default linker options
set(CMAKE_EXE_LINKER_FLAGS "-g -rdynamic -v")

# prepare data files
configure_file(../data/Dawn/Dawn.dsk       run/Dawn.dsk         COPYONLY)
configure_file(../data/Dawn/Dawn.germ      run/Dawn.germ        COPYONLY)
configure_file(../data/floppy/floppy144    run/floppy144        COPYONLY)
configure_file(../data/GVWin/GVWIN.DSK     run/GVWIN.DSK        COPYONLY)
configure_file(../data/GVWin/GVWIN.GRM     run/GVWIN.GRM        COPYONLY)
configure_file(../data/GVWin/NSINSTLR.DAT  run/NSINSTLR.DAT     COPYONLY)
configure_file(../data/GVWin21/GVWIN21.DSK run/GVWIN21.DSK      COPYONLY)
configure_file(../data/GVWin21/GVWIN21.GRM run/GVWIN21.GRM      COPYONLY)


# compile definition
add_definitions(-DBUILD_DIR="${CMAKE_BINARY_DIR}")

#
# platform dependant setting
#
if (${HOST_SYSTEM} STREQUAL Darwin)
  #
  # MacOS
  #
  # Compile for arm64
  set(CMAKE_OSX_ARCHITECTURES arm64)
  # SDK_PATH
  execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)  
  message(STATUS "SDK_PATH ${SDK_PATH}")
  # default include directories
  include_directories(SYSTEM
    ${SDK_PATH}/usr/include
    ${SDK_PATH}/usr/include/c++/v1
    /opt/local/include
    )
  # default link directories
  link_directories(/usr/lib /opt/local/lib)
else()
  message(FATAL_ERROR "Unknown system ${HOST_SYSTEM}")
endif()

# add subdirectory
add_subdirectory (agent	         ../build/agent)
add_subdirectory (bcdFile        ../build/bcdFile)
add_subdirectory (floppy      	 ../build/floppy)
add_subdirectory (guam-headless	 ../build/guam-headless)
add_subdirectory (main	         ../build/main)
add_subdirectory (mesa           ../build/mesa)
add_subdirectory (opcode         ../build/opcode)
add_subdirectory (print-bcdFile  ../build/print-bcdFile)
add_subdirectory (showType    	 ../build/showType)
add_subdirectory (tclMesa        ../build/tclMesa)
add_subdirectory (test           ../build/test)
add_subdirectory (util           ../build/util)
