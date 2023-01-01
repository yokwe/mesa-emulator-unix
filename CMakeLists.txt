cmake_minimum_required (VERSION 3.24.0)

project(mesa-emulator-unix VERSION 0.0.1 LANGUAGES CXX)

option(USE_QT6 "Use QT6" OFF) # QT5 or QT6
message(STATUS "USE_QT6    ${USE_QT6}")

#
# common setting
#
# host name and system
cmake_host_system_information(RESULT HOST_NAME QUERY HOSTNAME)
set(HOST_SYSTEM ${CMAKE_HOST_SYSTEM_NAME})
message(STATUS "NAME       ${HOST_NAME}")
message(STATUS "SYSTEM     ${HOST_SYSTEM}")

# qt setting
if (${USE_QT6})
  set(QT_VERSION 6)
else()
  set(QT_VERSION 5)
endif()
set(QT_NAME "Qt${QT_VERSION}")

message(STATUS "USE_QT6    ${USE_QT6}")
message(STATUS "QT_VERSION ${QT_VERSION}")
message(STATUS "QT_NAME    ${QT_NAME}")


# default c++ language level
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# default compiler options
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -g -O2 -fPIC")

# default linker options
SET(CMAKE_EXE_LINKER_FLAGS "-g -rdynamic -v")

# prepare logger configuration file
configure_file(data/debug.properties.in run/debug.properties @ONLY)

#
# platform dependant setting
#
if(${HOST_SYSTEM} STREQUAL FreeBSD)
  #
  # FreeBSD
  #
  # require for backtrace
  link_libraries (execinfo)
  # QT_DIR QT_INCLUDE
  set(QT_DIR     /usr/local/lib/qt${QT_VERSION})  
  set(QT_INCLUDE /usr/local/include/qt${QT_VERSION})
  # default include directories
  include_directories(SYSTEM
    /usr/include
    /usr/local/include
    ${QT_INCLUDE}
    ${QT_INCLUDE}/QtCore
    ${QT_INCLUDE}/QtGui
    )
  # default link directories
  link_directories(/usr/lib /usr/local/lib)
elseif (${HOST_SYSTEM} STREQUAL Darwin)
  #
  # MacOS
  #
  # Compile for arm64
  set(CMAKE_OSX_ARCHITECTURES arm64)
  # QT_DIR QT_INCLUDE
  set(QT_DIR     /opt/local/libexec/qt${QT_VERSION})
  set(QT_INCLUDE ${QT_DIR}/include)
  # default include directories
  include_directories(SYSTEM
    /usr/include
    /opt/local/include
    ${QT_INCLUDE}
    ${QT_INCLUDE}/QtCore
    ${QT_INCLUDE}/QtGui
    )
  # default link directories
  link_directories(/usr/lib /opt/local/lib)
else()
  message(FATAL_ERROR "Unknown system ${HOST_SYSTEM}")
endif()

message(STATUS "QT_DIR     ${QT_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# find package
find_package(${QT_NAME} COMPONENTS Core Gui REQUIRED)

# common library
link_libraries (log4cpp)
link_libraries(${QT_NAME}::Core ${QT_NAME}::Gui)

# add subdirectory
add_subdirectory (src/agent         build/agent)
#add_subdirectory (src/courier       build/courier)
#add_subdirectory (src/courierImpl   build/courierImpl)
#add_subdirectory (src/dumpSymbol    build/dumpSymbol)
add_subdirectory (src/guam-headless build/guam-headless)
#add_subdirectory (src/main          build/main)
add_subdirectory (src/mesa          build/mesa)
add_subdirectory (src/opcode        build/opcode)
#add_subdirectory (src/symbol        build/symbol)
add_subdirectory (src/test          build/test)
add_subdirectory (src/trace         build/trace)
add_subdirectory (src/util          build/util)
#add_subdirectory (src/xns           build/xns)
#add_subdirectory (src/xnsDump       build/xnsDump)
#add_subdirectory (src/xnsServer     build/xnsServer)
#add_subdirectory (src/xnsServerImpl build/xnsServerImpl)
