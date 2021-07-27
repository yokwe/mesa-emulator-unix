#
# Makefile
#

MODULE := main util test mesa agent opcode guam-headless

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

util:
	( cd src/util; make all )

main: util
	( cd src/main; make all )

run-main: main
	echo -n >tmp/debug.log
	tmp/build/main/main

test: util mesa agent opcode
	( cd src/test; make all )

run-test: test
	echo -n >tmp/debug.log
	tmp/build/test/test

mesa: util agent opcode
	( cd src/mesa; make all )

agent: util mesa
	( cd src/agent; make all )

opcode: util mesa
	( cd src/opcode; make all )

guam-headless: util mesa agent opcode
	( cd src/guam-headless; make all )

run-guam-headless-gvwin: guam-headless
	echo -n >tmp/debug.log
	tmp/build/guam-headless/guam-headless GVWin

run-guam-headless-dawn: guam-headless
	echo -n >tmp/debug.log
	tmp/build/guam-headless/guam-headless Dawn

gen-entry:
	awk -f data/map/genEntry.awk data/map/GermGuam.map >tmp/a

fix-permission:
	find . -type d -exec chmod 0755 {} \;
	find . -type f -exec chmod 0644 {} \;
