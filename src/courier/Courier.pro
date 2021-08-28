TARGET   = courier

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Courier.h   Type.h   Service.h   CHS.h
SOURCES += Courier.cpp Type.cpp Service.cpp CHS.cpp


###############################################
include(../common.pro)
