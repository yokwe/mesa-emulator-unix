#
# mesa
#

add_library(mesa STATIC)

target_sources(
	mesa
	PUBLIC
        Constant.h
        Function.h
        interrupt.h
        Memory.h
        MesaBasic.h
        guam.h
        Pilot.h
        processor.h
        timer.h
        Type.h
        Variable.h
    PRIVATE
        interrupt.cpp
        Memory.cpp
        guam.cpp
        Pilot.cpp
        processor.cpp
        timer.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
