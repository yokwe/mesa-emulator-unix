TEMPLATE = app

QMAKE_LFLAGS   += -g -rdynamic

# no gui
macx {
	CONFIG -= app_bundle
}


LIBS += ../../tmp/build/util/libutil.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a

# Input
SOURCES += main.cpp

###############################################
include(../common.pro)
