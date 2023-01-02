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
	cmake -B ${CMAKE_BUILD} -S . -G Ninja .

cmake-eclipse:
	cmake -B ${CMAKE_BUILD} -S . -G 'Eclipse CDT4 - Ninja' .
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${CMAKE_BUILD}/.project  .
	cp -p  ${CMAKE_BUILD}/.cproject .
	cp -rp ${CMAKE_BUILD}/.settings .

#
# Use qt5
#
cmake-eclipse-qt5:
	cmake -B ${CMAKE_BUILD} -S . -G 'Eclipse CDT4 - Ninja' -DUSE_QT6=OFF .
	@echo "copy eclipse setting files .project .cproject and .settings to current directory"
	cp -p  ${CMAKE_BUILD}/.project  .
	cp -p  ${CMAKE_BUILD}/.cproject .
	cp -rp ${CMAKE_BUILD}/.settings .

cmake-qt5:
	cmake -B ${CMAKE_BUILD} -S . -G Ninja -DUSE_QT6=OFF .


build:
	/usr/bin/time cmake --build ${CMAKE_BUILD}
	
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
	
run-main: clear-log
	${CMAKE_BUILD}/build/main/main
	
run-guam-headless-gvwin: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless ${CMAKE_BUILD} GVWin

run-guam-headless-gvwin21: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless ${CMAKE_BUILD} GVWin21

run-guam-headless-dawn: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless ${CMAKE_BUILD} Dawn


prepare-run-guam:
	@if [ ! -f ${CMAKE_BUILD}/run/xns-config.json ]; then \
		echo "copy xns-config.json to ${CMAKE_BUILD}/run"; \
		cp data/xns-config.json ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/GVWIN.DSK ]; then \
		echo "copy GVWIN.DSK to ${CMAKE_BUILD}/run"; \
		cp data/GVWin/GVWIN.DSK ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/GVWIN21.DSK ]; then \
		echo "copy GVWIN21.DSK to ${CMAKE_BUILD}/run"; \
		cp data/GVWin21/GVWIN21.DSK ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/Dawn.dsk ]; then \
		echo "copy Dawn.dsk to ${CMAKE_BUILD}/run"; \
		cp data/Dawn/Dawn.dsk ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/floppy144 ]; then \
		echo "copy floppy144 to ${CMAKE_BUILD}/run"; \
		cp data/floppy/floppy144 ${CMAKE_BUILD}/run ; \
	fi

# include legacy part
include Makefile-qmake
