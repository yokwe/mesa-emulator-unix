#
# Makefile
#

BUILD_DIR  := build
SOURCE_DIR := src
DATA_DIR   := data

export BUILD_DIR

LOG4CXX_CONFIGURATION := data/log4j-config.xml
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

src/util/trace.inc: src/util/trace.h data/gen-trace-inc.awk
	awk -f data/gen-trace-inc.awk src/util/trace.h >src/util/trace.inc

build: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build ${BUILD_DIR}

distclean: distclean-cmake distclean-macos

distclean-cmake:
	rm -rf ${BUILD_DIR}

distclean-macos:
	find . -type f -name '._*' -o -name '.DS_Store' -print -delete

main: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build build --target main
	
test: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build build --target test

guam-headless: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build build --target guam-headless

tclMesa:
	/usr/bin/time cmake --build build --target tclMesa

floppy:
	/usr/bin/time cmake --build build --target floppy

bcdFile: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build build --target bcdFile

print-bcdFile: src/util/Perf.inc src/util/trace.inc
	/usr/bin/time cmake --build build --target print-bcdFile

#
# run-XXX
#
prepare-log:
	mkdir -p ${BUILD_DIR}/run
	/bin/echo -n >${BUILD_DIR}/run/debug.log

run-test: test prepare-log
	${BUILD_DIR}/test/test

run-main: main prepare-log
	/usr/bin/time ${BUILD_DIR}/main/main

run-guam-headless: guam-headless
	/bin/echo -n >${BUILD_DIR}/run/guam-headless.log
	LOG4CXX_CONFIGURATION=data/log4j-config-guam-headless.xml /usr/bin/time ${BUILD_DIR}/guam-headless/guam-headless

run-tclMesa: tclMesa
	/bin/echo -n >${BUILD_DIR}/run/tclMesa.log
	LOG4CXX_CONFIGURATION=data/log4j-config-tclMesa.xml ${BUILD_DIR}/tclMesa/tclMesa

run-floppy: floppy
	/bin/echo -n >${BUILD_DIR}/run/floppy.log
	LOG4CXX_CONFIGURATION=data/log4j-config-floppy.xml ${BUILD_DIR}/floppy/floppy

run-print-bcdFile: print-bcdFile
	/bin/echo -n >${BUILD_DIR}/run/print-bcdFile.log
	LOG4CXX_CONFIGURATION=data/log4j-config-print-bcdFile.xml /usr/bin/time ${BUILD_DIR}/print-bcdFile/print-bcdFile

