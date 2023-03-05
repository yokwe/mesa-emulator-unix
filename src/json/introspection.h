//
// introspection.h
//

#pragma once

#include <string>
#include <type_traits>


namespace introspection {

std::string demangle(const char* mangled);
std::string enum_to_string(std::string type_name, int value);

template <typename T>
std::string enum_to_string(T value) {
	static_assert(std::is_enum<T>::value);

	std::string type_name = demangle(typeid(T).name());
	return enum_to_string(type_name, (int)value);
}

}
