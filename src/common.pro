# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += .

LIBS += -llog4cpp

CONFIG += c++1z

macx {
	INCLUDEPATH += /opt/local/include
	LIBS        += -L/opt/local/lib
	
	# build for build target
	QMAKE_MACOSX_DEPLOYMENT_TARGET = $$system(sw_vers -productVersion)
	# or build for specific version
	# QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0
	
	# Use Link Time Optimization
	QMAKE_CXXFLAGS += -flto
	QMAKE_LFLAGS   += -flto
	
	# Profiler
	defined(PROF,var) {
		message("PROF macx")
	}
} else:freebsd {
	LIBS += -lexecinfo
	
	# Use Link Time Optimization
	QMAKE_CXXFLAGS += -flto
	QMAKE_LFLAGS   += -flto
	
	# Profiler
	defined(PROF,var) {
		message("PROF freebsd")
	}
} else {
	error("Unexpected")
}


QMAKE_CXXFLAGS += -Werror -g

QMAKE_LFLAGS   += -g -rdynamic -v

DESTDIR     = ../../tmp/build/$$TARGET
OBJECTS_DIR = ../../tmp/build/$$TARGET
MOC_DIR     = ../../tmp/build/$$TARGET
RCC_DIR     = ../../tmp/build/$$TARGET
UI_DIR      = ../../tmp/build/$$TARGET
