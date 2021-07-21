TARGET   = util

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Util.h   Preference.h
SOURCES += Util.cpp Preference.cpp


###############################################
include(../common.pro)
