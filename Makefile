#
# Makefile
#

MODULE := main test guam-headless \
          util mesa agent opcode trace symbols xns xnsServer

.PHONY: all clean distclean gamke fix-permission
.PHONY:     main     test     xnsServer
.PHONY: run-main run-test run-xnsServer
.PHONY:     guam-headless prepare-run-guam
.PONEY: run-guam-headless-gvwin run-guam-headless-gvwin21 run-guam-headless-dawn
.PHONY: util mesa symbols xns

all:
	echo "all"
	@echo "--------"
	env
	@echo "--------"
	@echo "MODULE $(MODULE)!"

clean:
	echo "PATH = $(PATH)"
	rm -rf tmp/build
	@echo "MODULE $(MODULE)!"
	@for i in $(MODULE); do echo "mkdir -p tmp/build/$$i"; mkdir -p tmp/build/$$i; done

distclean: clean
	rm -f  src/*/Makefile
	rm -f  src/*/Makefile.Debug
	rm -f  src/*/Makefile.Release
	rm -f  src/*/object_script.*.Debug
	rm -f  src/*/object_script.*.Release

qmake:
	@echo "MODULE $(MODULE)!"
	@for i in $(MODULE); do echo "cd src/$$i; qmake"; (cd src/$$i; qmake); done

fix-permission:
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;

util:
	( cd src/util;   make all )

main: util
	( cd src/main;   make all )


mesa: util
	( cd src/agent;  make all )
	( cd src/opcode; make all )
	( cd src/mesa;   make all )
	( cd src/trace;  make all )

test: util mesa
	( cd src/test;   make all )

symbols: util mesa
	( cd src/symbols; make all )

xns: util
	( cd src/xns; make all )
	
xnsServer: xns
	( cd src/xnsServer; make all )

guam-headless: util mesa
	( cd src/guam-headless; make all )

prepare-run-guam:
	@if [ ! -d tmp/run ]; then \
		echo "create tmp/run"; \
		mkdir -p tmp/run ; \
	fi
	@if [ ! -f tmp/run/debug.properties ]; then \
		echo "copy debug.properties"; \
		cp data/debug.properties tmp/run ; \
	fi
	@if [ ! -f tmp/run/setting.json ]; then \
		echo "copy setting.json"; \
		cp data/setting.json tmp/run ; \
	fi
	@if [ ! -f tmp/run/xns-config.json ]; then \
		echo "copy xns-config.json"; \
		cp data/xns-config.json tmp/run ; \
	fi
	@if [ ! -f tmp/run/GVWIN.DSK ]; then \
		echo "copy GVWIN.DSK"; \
		cp data/GVWin/GVWIN.DSK tmp/run ; \
	fi
	@if [ ! -f tmp/run/GVWIN21.DSK ]; then \
		echo "copy GVWIN21.DSK"; \
		cp data/GVWin21/GVWIN21.DSK tmp/run ; \
	fi
	@if [ ! -f tmp/run/Dawn.dsk ]; then \
		echo "copy Dawn.dsk"; \
		cp data/Dawn/Dawn.dsk tmp/run ; \
	fi
	@if [ ! -f tmp/run/floppy144 ]; then \
		echo "copy floppy144"; \
		cp data/floppy/floppy144 tmp/run ; \
	fi
	
run-main: main prepare-run-guam
	echo -n >tmp/run/debug.log
	tmp/build/main/main

run-xnsServer: xnsServer
	echo -n >tmp/run/debug.log
	tmp/build/xnsServer/xnsServer

run-test: test
	echo -n >tmp/run/debug.log
	tmp/build/test/test

run-guam-headless-gvwin: guam-headless prepare-run-guam
	echo -n >tmp/run/debug.log
	tmp/build/guam-headless/guam-headless GVWin

run-guam-headless-gvwin21: guam-headless prepare-run-guam
	echo -n >tmp/run/debug.log
	tmp/build/guam-headless/guam-headless GVWin21

run-guam-headless-dawn: guam-headless prepare-run-guam
	echo -n >tmp/run/debug.log
	tmp/build/guam-headless/guam-headless Dawn
