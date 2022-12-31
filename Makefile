#
# Makefile
#

.PHONY: all clean cmake build distclean distclean-qmake fix-permission

all:
	echo "all"
	cmake --build . --target help

clean:
	cmake --build . --target clean

cmake:
	cmake -G 'Eclipse CDT4 - Ninja' -DUSE_QT6=OFF .

build:
	time cmake --build .
	
distclean: distclean-ninja distclean-cmake distclean-qmake
	rm -rf build
	rm -rf tmp/build

distclean-ninja:
	rm -rf .ninja_deps .ninja_log build.ninja

distclean-cmake:
	rm -rf cmake_install.cmake CMakeCache.txt CMakeFiles

distclean-qmake:
	rm -f  src/*/Makefile
	rm -f  src/*/Makefile.Debug
	rm -f  src/*/Makefile.Release
	rm -f  src/*/object_script.*.Debug
	rm -f  src/*/object_script.*.Release
	rm -f  src/*/.qmake.stash

fix-permission:
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;

clear-log:
	echo -n >tmp/run/debug.log

run-test: clear-log
	tmp/build/${HOSTNAME}/test/test
	

# include legacy part
include Makefile-qmake
