#
# Makefile
#

BUILD_DIR  := build
SOURCE_DIR := src
DATA_DIR   := data

export BUILD_DIR

LOG4CXX_CONFIGURATION := ${BUILD_DIR}/run/log4j-config.xml
export LOG4CXX_CONFIGURATION


.PHONY: all clean help cmake build distclean distclean-cmake distclean-macos
.PHONY: main test guam-headless
.PHONY: run-main run-test run-guam-headless

all:
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

src/util/Perf.inc: src/util/Perf.h data/gen-perf-inc.awk
	awk -f data/gen-perf-inc.awk src/util/Perf.h >src/util/Perf.inc

build: src/util/Perf.inc
	/usr/bin/time cmake --build ${BUILD_DIR}

distclean: distclean-cmake distclean-macos

distclean-cmake:
	rm -rf ${BUILD_DIR}

distclean-macos:
	find . -type f -name '._*' -print -delete

main: src/util/Perf.inc
	/usr/bin/time cmake --build build --target main
	
test: src/util/Perf.inc
	/usr/bin/time cmake --build build --target test

guam-headless: src/util/Perf.inc
	/usr/bin/time cmake --build build --target guam-headless

tclGuam:
	/usr/bin/time cmake --build build --target tclGuam

	
#
# run-XXX
#
prepare-log:
	mkdir -p ${BUILD_DIR}/run
	echo -n >${BUILD_DIR}/run/debug.log

run-main: main prepare-log
	/usr/bin/time ${BUILD_DIR}/main/main

run-guam-headless: guam-headless prepare-log
	/usr/bin/time ${BUILD_DIR}/guam-headless/guam-headless

run-test: test prepare-log
	${BUILD_DIR}/test/test

run-tclGuam: tclGuam prepare-log
	${BUILD_DIR}/tclGuam/tclGuam
