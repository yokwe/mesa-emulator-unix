#
# Makefile
#

all:
	echo "all"
	@echo "--------"
	env
	@echo "--------"
	
clean:
	echo "PATH = $(PATH)"
	rm -rf tmp/build
	mkdir  tmp/build
	mkdir  tmp/build/main
	mkdir  tmp/build/util

distclean: clean
	rm -f  src/*/Makefile
	rm -f  src/*/Makefile.Debug
	rm -f  src/*/Makefile.Release
	rm -f  src/*/object_script.*.Debug
	rm -f  src/*/object_script.*.Release

qmake:
	( cd src/util; qmake )
	( cd src/main; qmake )

util:
	( cd src/util; make all )

main: util
	( cd src/main; make all )

run-main: main
	echo -n >tmp/debug.log
	tmp/build/main/main
