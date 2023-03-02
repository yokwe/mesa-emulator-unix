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

cmake:
	cmake -B ${BUILD_DIR} -S . -G Ninja .

cmake-eclipse:
	cmake -B ${BUILD_DIR} -S . -G 'Eclipse CDT4 - Ninja' .
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${BUILD_DIR}/.project  .
	cp -p  ${BUILD_DIR}/.cproject .
	cp -rp ${BUILD_DIR}/.settings .

#
# Use qt5
#
cmake-eclipse-qt5:
	cmake -B ${BUILD_DIR} -S . -G 'Eclipse CDT4 - Ninja' -DUSE_QT6=OFF .
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${BUILD_DIR}/.project  .
	cp -p  ${BUILD_DIR}/.cproject .
	cp -rp ${BUILD_DIR}/.settings .

cmake-qt5:
	cmake -B ${BUILD_DIR} -S . -G Ninja -DUSE_QT6=OFF .


build:
	/usr/bin/time cmake --build ${BUILD_DIR}
	
distclean: distclean-qmake distclean-eclipse distclean-cmake clean-macos

distclean-qmake:
	rm -f  src/*/Makefile
	rm -f  src/*/Makefile.Debug
	rm -f  src/*/Makefile.Release
	rm -f  src/*/object_script.*.Debug
	rm -f  src/*/object_script.*.Release
	rm -f  src/*/.qmake.stash

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

check-bpf:
	@if [ -c /dev/bpf ]; then \
	if [ ! -w /dev/bpf -o ! -r /dev/bpf ]; then \
		ls -l /dev/bpf; \
		echo "sudo chmod 660 /dev/bpf"; \
		sudo chmod 660 /dev/bpf; \
		ls -l /dev/bpf; \
	fi ; \
	fi
	@if [ -c /dev/bpf0 ]; then \
	if [ ! -w /dev/bpf0 -o ! -r /dev/bpf0 ]; then \
		ls -l /dev/bpf0; \
		echo "sudo chmod 660 /dev/bpf0"; \
		sudo chmod 660 /dev/bpf0; \
		ls -l /dev/bpf0; \
	fi ; \
	fi
	@if [ -c /dev/bpf1 ]; then \
	if [ ! -w /dev/bpf1 -o ! -r /dev/bpf1 ]; then \
		ls -l /dev/bpf1; \
		echo "sudo chmod 660 /dev/bpf1"; \
		sudo chmod 660 /dev/bpf1; \
		ls -l /dev/bpf1; \
	fi ; \
	fi
	@if [ -c /dev/bpf2 ]; then \
	if [ ! -w /dev/bpf2 -o ! -r /dev/bpf2 ]; then \
		ls -l /dev/bpf2; \
		echo "sudo chmod 660 /dev/bpf2"; \
		sudo chmod 660 /dev/bpf2; \
		ls -l /dev/bpf2; \
	fi ; \
	fi
	@if [ -c /dev/bpf3 ]; then \
	if [ ! -w /dev/bpf3 -o ! -r /dev/bpf3 ]; then \
		ls -l /dev/bpf3; \
		echo "sudo chmod 660 /dev/bpf3"; \
		sudo chmod 660 /dev/bpf3; \
		ls -l /dev/bpf3; \
	fi ; \
	fi
	
run-test: clear-log
	${BUILD_DIR}/build/test/test
	
run-main: clear-log
	/usr/bin/time ${BUILD_DIR}/build/main/main <tmp/a
	
run-json: clear-log
	@if [ ${HOST_OS} = "Darwin" ]; then \
		clang -Xclang -ast-dump=json -fsyntax-only \
		    -I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1 \
		    -I /Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/include \
		    -I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include \
		    -I /Library/Developer/CommandLineTools/usr/include \
			-I /opt/local/include \
			-I /opt/local/libexec/qt6/include \
			-I /opt/local/libexec/qt6/include/QtCore \
			-std=c++17 src/json/dummy.cpp | \
			/usr/bin/time ${BUILD_DIR}/build/json/json; \
	fi
	@if [ ${HOST_OS} = "FreeBSD" ]; then \
		clang -Xclang -ast-dump=json -fsyntax-only \
			-I /usr/local/include \
			-I /usr/local/include/qt6 \
			-I /usr/local/include/qt6/QtCore \
			-std=c++17 src/json/dummy.cpp | \
			/usr/bin/time ${BUILD_DIR}/build/json/json; \
	fi
	
run-dumpSymbol: clear-log
	${BUILD_DIR}/build/dumpSymbol/dumpSymbol tmp/dumpSymbol/*.bcd

run-xnsDump: clear-log check-bpf
	${BUILD_DIR}/build/xnsDump/xnsDump

run-xnsServer: clear-log check-bpf
	${BUILD_DIR}/build/xnsServer/xnsServer

run-guam-headless-gvwin: prepare-run-guam clear-log
	${BUILD_DIR}/build/guam-headless/guam-headless GVWin

run-guam-headless-gvwin21: prepare-run-guam clear-log
	${BUILD_DIR}/build/guam-headless/guam-headless GVWin21

run-guam-headless-dawn: prepare-run-guam clear-log
	${BUILD_DIR}/build/guam-headless/guam-headless Dawn


prepare-run-guam:
	@if [ ! -f ${BUILD_DIR}/run/GVWIN.DSK ]; then \
		echo "copy GVWIN.DSK to ${BUILD_DIR}/run"; \
		cp data/GVWin/GVWIN.DSK ${BUILD_DIR}/run ; \
	fi
	@if [ ! -f ${BUILD_DIR}/run/GVWIN21.DSK ]; then \
		echo "copy GVWIN21.DSK to ${BUILD_DIR}/run"; \
		cp data/GVWin21/GVWIN21.DSK ${BUILD_DIR}/run ; \
	fi
	@if [ ! -f ${BUILD_DIR}/run/Dawn.dsk ]; then \
		echo "copy Dawn.dsk to ${BUILD_DIR}/run"; \
		cp data/Dawn/Dawn.dsk ${BUILD_DIR}/run ; \
	fi
	@if [ ! -f ${BUILD_DIR}/run/floppy144 ]; then \
		echo "copy floppy144 to ${BUILD_DIR}/run"; \
		cp data/floppy/floppy144 ${BUILD_DIR}/run ; \
	fi
