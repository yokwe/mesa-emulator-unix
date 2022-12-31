#
# Makefile
#

CMAKE_BUILD := tmp/cmake/${HOSTNAME}

.PHONY: all clean cmake build distclean distclean-qmake fix-permission

all:
	@echo "all"
	@echo "CMAKE_BUILD ${CMAKE_BUILD}"

clean:
	cmake --build ${CMAKE_BUILD} --target clean

help:
	cmake --build ${CMAKE_BUILD} --target help

cmake:
	cmake -B ${CMAKE_BUILD} -S . -G 'Eclipse CDT4 - Ninja' -DUSE_QT6=OFF .
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${CMAKE_BUILD}/.project  .
	cp -p  ${CMAKE_BUILD}/.cproject .
	cp -rp ${CMAKE_BUILD}/.settings .

build:
	time cmake --build ${CMAKE_BUILD}
	
distclean: distclean-qmake
	rm -rf build
	rm -rf ${CMAKE_BUILD}

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
	mkdir -p ${CMAKE_BUILD}/run
	echo -n >${CMAKE_BUILD}/run/debug.log

run-test: clear-log
	${CMAKE_BUILD}/build/test/test
	

# include legacy part
include Makefile-qmake
