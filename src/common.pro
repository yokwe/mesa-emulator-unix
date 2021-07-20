# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += .

macx {
	INCLUDEPATH += /opt/local/include
	LIBS        += -L/opt/local/lib
}

QMAKE_CXXFLAGS += -std=c++17 -Wall -Werror -g

QMAKE_LFLAGS   += -g -rdynamic

DESTDIR     = ../../tmp/build/$$TARGET
OBJECTS_DIR = ../../tmp/build/$$TARGET
MOC_DIR     = ../../tmp/build/$$TARGET
RCC_DIR     = ../../tmp/build/$$TARGET
UI_DIR      = ../../tmp/build/$$TARGET
