cmake_minimum_required (VERSION 3.24.0)

project(mesa-emulator-unix VERSION 0.0.1 LANGUAGES CXX)

option(USE_QT6 "Use QT6" OFF) # QT5 or QT6

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

#
# platform dependant setting
#
if(${HOST_SYSTEM} STREQUAL FreeBSD)
  #
  # FreeBSD
  #
  
  # default include directories
  include_directories(SYSTEM /usr/include /usr/local/include /usr/include/c++/v1)
  
  # default link directories
  link_directories(/usr/lib /usr/local/lib /usr/local/lib/qt${QT_VERSION})
  # default library
  link_libraries (execinfo)
  
  set(QT_DIR /usr/local/lib/qt${QT_VERSION})
else()
  message(FATAL_ERROR "Unknown system ${HOST_SYSTEM}")
endif()

message(STATUS "QT_DIR     ${QT_DIR}")

include_directories(SYSTEM
  /usr/local/include/qt${QT_VERSION}
  /usr/local/include/qt${QT_VERSION}/QtCore
  /usr/local/include/qt${QT_VERSION}/QtWidgets
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(${QT_NAME} REQUIRED COMPONENTS Core)
find_package(${QT_NAME} REQUIRED COMPONENTS Widgets)

if (${QT_VERSION} EQUAL 6)
  qt_standard_project_setup()
endif()


# common library
link_libraries (log4cpp)
link_libraries (Qt${QT_VERSION}Gui Qt${QT_VERSION}Core)


# add subdirectory
add_subdirectory (src/util   tmp/build/${HOST_NAME}/util)
add_subdirectory (src/mesa   tmp/build/${HOST_NAME}/mesa)
add_subdirectory (src/opcode tmp/build/${HOST_NAME}/opcode)
add_subdirectory (src/agent  tmp/build/${HOST_NAME}/agent)
add_subdirectory (src/trace  tmp/build/${HOST_NAME}/trace)
add_subdirectory (src/test   tmp/build/${HOST_NAME}/test)
