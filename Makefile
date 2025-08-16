#
# Makefile
#

#   SHELL       path of bash
#   HOSTNAME    name of host
#   BUILD_DIR   path of build
#   LOG_CONFIG  path of logger configuration file

SHELL := $(shell which bash)

HOSTNAME := $(shell uname -n)

HOST_OS := $(shell uname -s)

BUILD_DIR := build

SOURCE_DIR := src3

LOG4CXX_CONFIGURATION := ${BUILD_DIR}/run/log4j-config.xml
export LOG4CXX_CONFIGURATION


.PHONY: all clean cmake build distclean distclean-qmake fix-permission

all:
	@echo "all"
	@echo "BUILD_DIR             ${BUILD_DIR}"
	@echo "SOURCE_DIR            ${SOURCE_DIR}"
	@echo "LOG4CXX_CONFIGURATION ${LOG4CXX_CONFIGURATION}"

#
# cmake related targets
#
clean:
	cmake --build ${BUILD_DIR} --target clean

help:
	cmake --build ${BUILD_DIR} --target help

cmake: distclean-cmake
	mkdir -p ${BUILD_DIR}; cd ${BUILD_DIR}; cmake ../${SOURCE_DIR} -G Ninja

build:
	/usr/bin/time cmake --build ${BUILD_DIR}

main:
	/usr/bin/time cmake --build build --target main
	
test:
	/usr/bin/time cmake --build build --target test

#
# maintenance
#
distclean: distclean-cmake distclean-macos

distclean-cmake:
	rm -rf ${BUILD_DIR}

distclean-macos:
	find . -type f -name '._*' -print -delete

#
# run-XXX
#
prepare-log:
	mkdir -p ${BUILD_DIR}/run
	echo -n >${BUILD_DIR}/run/debug.log

run-main: main prepare-log
	/usr/bin/time ${BUILD_DIR}/main/main

run-test: test prepare-log
	${BUILD_DIR}/test/test
	