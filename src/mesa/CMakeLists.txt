#
# mesa
#

add_library(mesa STATIC)

target_sources(
	mesa
	PUBLIC
        Constant.h
        Function.h
        InterruptThread.h
        Memory.h
        MesaBasic.h
        MesaProcessor.h
        Pilot.h
        ProcessorThread.h
        timer.h
        Type.h
        Variable.h
    PRIVATE
        InterruptThread.cpp
        Memory.cpp
        MesaProcessor.cpp
        Pilot.cpp
        ProcessorThread.cpp
        timer.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
