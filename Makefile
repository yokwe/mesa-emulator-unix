#
# Makefile
#

# BSDMakefile or GNUMakefile will define SHELL HOSTNAME BUILD_DIR LOG_CONFIG
#   SHELL       path of bash
#   HOSTNAME    name of host
#   BUILD_DIR   path of build
#   LOG_CONFIG  path of logger configuration file



.PHONY: all clean cmake build distclean distclean-qmake fix-permission

all:
	@echo "all"
	@echo "BUILD_DIR ${BUILD_DIR}"

clean:
	cmake --build ${BUILD_DIR} --target clean

help:
	cmake --build ${BUILD_DIR} --target help

cmake: distclean-cmake
	mkdir -p ${BUILD_DIR}; cd ${BUILD_DIR}; cmake ../src2 -G Ninja

cmake-eclipse: distclean-cmake
	mkdir -p ${BUILD_DIR}; cd ${BUILD_DIR}; cmake ../src2 -G 'Eclipse CDT4 - Ninja'
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${BUILD_DIR}/.project  .
	cp -p  ${BUILD_DIR}/.cproject .
	cp -rp ${BUILD_DIR}/.settings .


build:
	/usr/bin/time cmake --build ${BUILD_DIR}
	
distclean: distclean-eclipse distclean-cmake clean-macos

distclean-eclipse:
	rm -rf .project .cproject .settings build

distclean-cmake:
	rm -rf ${BUILD_DIR}

distclean-macos:
	find . -type f -name '._*' -print -delete

fix-permission:
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;

clear-log:
	mkdir -p ${BUILD_DIR}/run
	echo -n >${BUILD_DIR}/run/debug.log


run-main: main clear-log
	/usr/bin/time ${BUILD_DIR}/main/main

run-test: test clear-log
	${BUILD_DIR}/test/test

main:
	/usr/bin/time cmake --build build --target main
	
test:
	/usr/bin/time cmake --build build --target test
	