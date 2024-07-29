#
# main
#

add_executable(main)

target_sources(
	main
	PRIVATE
		main.cpp
)

add_dependencies(main util mesa)

target_link_libraries (main mesa util)
