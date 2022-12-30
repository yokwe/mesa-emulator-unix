cmake_minimum_required (VERSION 3.24.0)

project(mesa-emulator-unix VERSION 0.0.1 LANGUAGES CXX)

option(USE_QT6 "Use QT6" OFF) # QT5 or QT6

#
# common setting
#
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

#
# platform dependant setting
#
message(STATUS "SYSTEM     ${CMAKE_HOST_SYSTEM_NAME}")
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL FreeBSD)
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
  message(FATAL_ERROR "Unknown system ${CMAKE_HOST_SYSTEM_NAME}")
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


# custom target
add_custom_target(dist-clean rm -rf .ninja_deps .ninja_logs build.ninja cmake_install.cmake CMakeCache.txt CMakeFiles tmp/build)
add_custom_target(run-test LOG4CXX_CONFIG=data/log4cxx.xml tmp/build/test/test)


add_subdirectory (src/util   tmp/build/util)
add_subdirectory (src/mesa   tmp/build/mesa)
add_subdirectory (src/opcode tmp/build/opcode)
add_subdirectory (src/agent  tmp/build/agent)
add_subdirectory (src/trace  tmp/build/trace)
add_subdirectory (src/test   tmp/build/test)

