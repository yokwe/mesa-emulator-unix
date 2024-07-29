#
# mesa
#

add_library(mesa SHARED)

target_sources(
	mesa
	PUBLIC
		Constant.h
		Function.h
		Memory.h
		Pilot.h
		PrincOps.h
		Type.h
		Variable.h
	PRIVATE
		Dummy.cpp
		Memory.cpp
		PrincOps.cpp
)

add_dependencies(mesa util)

target_link_libraries (mesa util)
