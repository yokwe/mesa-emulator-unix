//
// inrospection.cpp
//

#include <map>
#include <unordered_map>

#include <cxxabi.h>

#include "introspection.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("introspection");


namespace introspection {
// start of introspection namespace

std::string demangle(const char* mangled) {
	char buffer[512];
	size_t length(sizeof(buffer));
	int status(0);

	char* demangled = __cxxabiv1::__cxa_demangle(mangled, buffer, &length, &status);
	if (status != 0) {
		logger.warn("demange %d %s", status, mangled);
	}

	std::string ret(demangled);
	return ret;
}


using enum_item_t  = std::pair<int, std::string>;
using enum_items_t = std::vector<enum_item_t>;


void initialize_enum_map();

struct enum_type_t {
	std::string   m_type_name;
	enum_items_t  m_items;

	void type_name(const std::string& type_name) {
		m_type_name = type_name;
	}

	void add(int value, const std::string& name) {
		m_items.emplace_back(value, name);
	}

	std::string to_string(int value) const {
		for(auto i = m_items.cbegin(); i != m_items.cend(); i++) {
			if (i->first == value) {
				return i->second;
			}
		}
		// not found
		logger.info("Unknown enum value  %s  %d", m_type_name, value);
		return std::to_string(value);
	}
};
struct enum_map_t {
	bool initialized = false;
	std::map<std::string, enum_type_t> map;

	bool empty() {
		return map.empty();
	}

	std::string to_string(const std::string& type_name, int value) {
		if (!initialized) {
			initialize_enum_map();
			initialized = true;
		}
		const auto i = map.find(type_name);
		if (i == map.cend()) {
			// not found
			logger.info("Unknown enum value  %s  %d", type_name, value);
			return std::to_string(value);
		} else {
			return i->second.to_string(value);
		}
	}

	void add(const std::string& type_name, int value, const std::string& name) {
		auto i = map.find(type_name);
		if (i == map.cend()) {
			enum_type_t type;
			type.type_name(type_name);
			type.add(value, name);
			map[type_name] = type;
		} else {
			i->second.add(value, name);
		}

	}
};

enum_map_t enum_map;

#define add(type_name, value) enum_map.add(#type_name, type_name::value, #value)
void initialize_enum_map() {
	// add(LevelVKeys::KeyName, null);

#include "introspection_enum.cpp"
}


std::string enum_to_string(std::string type_name, int value) {
	return enum_map.to_string(type_name, value);
}


// end of introspection namespace
}

