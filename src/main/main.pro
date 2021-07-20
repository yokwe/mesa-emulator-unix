TEMPLATE = app

QMAKE_LFLAGS   += -g

unix {
	QMAKE_LFLAGS   += -rdynamic
}


LIBS += ../../tmp/build/util/libutil.a

LIBS += -llog4cpp

POST_TARGETDEPS += ../../tmp/build/util/libutil.a

# Input
SOURCES += main.cpp

###############################################
include(../common.pro)
