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
	
run-guam-headless-gvwin: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless GVWin

run-guam-headless-gvwin21: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless GVWin21

run-guam-headless-dawn: prepare-run-guam clear-log
	${CMAKE_BUILD}/build/guam-headless/guam-headless Dawn


prepare-run-guam:
	@if [ ! -d ${CMAKE_BUILD}/run ]; then \
		echo "create tmp/run"; \
		mkdir -p ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/debug.properties ]; then \
		echo "copy debug.properties"; \
		cp data/debug.properties ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/setting.json ]; then \
		echo "copy setting.json"; \
		cp data/setting.json ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/xns-config.json ]; then \
		echo "copy xns-config.json"; \
		cp data/xns-config.json ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/GVWIN.DSK ]; then \
		echo "copy GVWIN.DSK"; \
		cp data/GVWin/GVWIN.DSK ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/GVWIN21.DSK ]; then \
		echo "copy GVWIN21.DSK"; \
		cp data/GVWin21/GVWIN21.DSK ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/Dawn.dsk ]; then \
		echo "copy Dawn.dsk"; \
		cp data/Dawn/Dawn.dsk ${CMAKE_BUILD}/run ; \
	fi
	@if [ ! -f ${CMAKE_BUILD}/run/floppy144 ]; then \
		echo "copy floppy144"; \
		cp data/floppy/floppy144 ${CMAKE_BUILD}/run ; \
	fi

# include legacy part
include Makefile-qmake
