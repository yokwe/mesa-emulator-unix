#
# mesa
#

add_library(mesa SHARED)

target_sources(
	mesa
	PUBLIC
		Constant.h
		Debug.h
		Function.h
		Memory.h
		Perf.h
		Pilot.h
		PrincOps.h
		Type.h
		Variable.h
	PRIVATE
		Dummy.cpp
		Memory.cpp
		Perf.cpp
		PrincOps.cpp
		Variable.cpp
)

add_dependencies(mesa util)

target_link_libraries (mesa util)
