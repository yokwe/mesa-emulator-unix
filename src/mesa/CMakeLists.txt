#
# mesa
#

add_library(mesa STATIC)

target_sources(
mesa
PUBLIC
        Constant.h
        display.h
        Function.h
        guam_config.h
        memory.h
        MesaBasic.h
        guam.h
        Pilot.h
        processor_thread.h
        Type.h
        Variable.h
PRIVATE
        display.cpp
        guam_config.cpp
        memory.cpp
        guam.cpp
        Pilot.cpp
        processor_thread.cpp
        Type.cpp
        Variable.cpp
)

add_dependencies(mesa util)
