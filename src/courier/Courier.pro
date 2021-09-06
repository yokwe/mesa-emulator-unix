TARGET   = courier

TEMPLATE = lib
CONFIG  += staticlib

# Input
HEADERS += Type.h   Protocol.h   Service.h   CHS.h   DataStream.h   TextStream.h
SOURCES += Type.cpp Protocol.cpp Service.cpp CHS.cpp DataStream.cpp TextStream.cpp


HEADERS += Authentication1.h   Authentication3.h   BulkData.h   Clearinghouse2.h   Clearinghouse3.h
SOURCES += Authentication1.cpp Authentication3.cpp BulkData.cpp Clearinghouse2.cpp Clearinghouse3.cpp


###############################################
include(../common.pro)
