cmake_minimum_required (VERSION 3.24.0)

project(mesa-emulator-unix VERSION 0.0.1 LANGUAGES CXX)

option(USE_QT6 "Use QT6" ON) # default is QT6

#
# common setting
#
# host name and system
cmake_host_system_information(RESULT HOST_NAME QUERY HOSTNAME)
set(HOST_SYSTEM ${CMAKE_HOST_SYSTEM_NAME})
message(STATUS "NAME       ${HOST_NAME}")
message(STATUS "SYSTEM     ${HOST_SYSTEM}")


# default c++ language level
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions("__cplusplus=201703L")

# default compiler options
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -g -O2 -fPIC")

# default linker options
set(CMAKE_EXE_LINKER_FLAGS "-g -rdynamic -v")

# prepare data files
message(STATUS "Copy log4j-config")
configure_file(../data2/log4j-config.xml run/log4j-config.xml @ONLY)

# compile definition
# STD_SPRINTF_STOP_COMPILE_IF_VALUE_IS_QSTRING - stop compile if one of argument of std_sprintf is QString
add_compile_definitions("STD_SPRINTF_STOP_COMPILE_IF_VALUE_IS_QSTRING")

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
add_subdirectory (util	 ../build/util)
add_subdirectory (mesa	 ../build/mesa)
add_subdirectory (main   ../build/main)
