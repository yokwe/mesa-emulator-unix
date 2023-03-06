//
// JSON
//

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <istream>
#include <regex>
#include <optional>

#include "../util/Util.h"

namespace json {


//
// utility function
//
std::string glob_to_regex_string(const std::string& glob);

std::regex  glob_to_regex(const std::string& glob);

template<typename ... Args>
std::regex  glob_to_regex(Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex_string(e) + ")");
	}
	return std::regex(string.substr(1));
}

std::string json_quote(const std::string& string);


//
// data
//
class token_t {
	enum class Type {
		STRING, BOOL, SIGNED_INTEGER, UNSIGNED_INTEGER, FLOAT, NULL_TYPE,
		START_OBJECT, END_OBJECT,
		START_ARRAY,  END_ARRAY,
	};

	static const std::string& to_string(Type type) {
		static std::string type_string          ("STRING");
		static std::string type_bool            ("BOOL");
		static std::string type_signed_integer  ("SIGNED_INTEGER");
		static std::string type_unsigned_integer("UNSIGNED_INTEGER");
		static std::string type_float           ("FLOAT");
		static std::string type_null            ("NULL_TYPE");
		static std::string type_start_object    ("START_OBJECT");
		static std::string type_end_object      ("END_OBJECT");
		static std::string type_start_array     ("START_ARRAY");
		static std::string type_end_array       ("END_ARRAY");
		static std::string type_error           ("*ERROR*");

		switch(type) {
		case Type::STRING:
			return type_string;
		case Type::BOOL:
			return type_bool;
		case Type::SIGNED_INTEGER:
			return type_signed_integer;
		case Type::UNSIGNED_INTEGER:
			return type_unsigned_integer;
		case Type::FLOAT:
			return type_float;
		case Type::NULL_TYPE:
			return type_null;
		case Type::START_OBJECT:
			return type_start_object;
		case Type::END_OBJECT:
			return type_end_object;
		case Type::START_ARRAY:
			return type_start_array;
		case Type::END_ARRAY:
			return type_end_array;
		default:
			assert(false);
			return type_error;
		}
	}

	std::string  m_path;
	Type         m_type;
	std::string  m_name;
	std::string  m_value;
	//
	std::int64_t m_int64_value;
	double       m_double_value;

	token_t(const std::string& path_, Type type_, const std::string& name_, const std::string& value_, std::int64_t int64_value_, double double_value_) :
		m_path(path_), m_type(type_), m_name(name_), m_value(value_), m_int64_value(int64_value_), m_double_value(double_value_) {}

public:
	token_t() :
		m_path("*NULL*"),
		m_type(Type::NULL_TYPE),
		m_name("*NULL*"),
		m_value("*NULL*"),
		m_int64_value(0),
		m_double_value(0) {}

	// item
	static token_t make_float(const std::string& path_, const std::string& name_, const std::string& value_, const double double_value_) {
		return token_t(path_, Type::FLOAT, name_, value_, 0, double_value_);
	}
	static token_t make_singned_integer(const std::string& path_, const std::string& name_, const int64_t value_) {
		return token_t(path_, Type::SIGNED_INTEGER, name_, std::to_string(value_), value_, 0);
	}
	static token_t make_unsigned_integer(const std::string& path_, const std::string& name_, const uint64_t value_) {
		return token_t(path_, Type::UNSIGNED_INTEGER, name_, std::to_string(value_), (int64_t)value_, 0);
	}
	static token_t make_string(const std::string& path_, const std::string& name_, const std::string& value_) {
		return token_t(path_, Type::STRING, name_, value_, 0, 0);
	}
	static token_t make_bool(const std::string& path_, const std::string& name_, const bool value_) {
		return token_t(path_, Type::BOOL, name_, value_ ? "TRUE" : "FALSE", (value_ ? 1 : 0), 0);
	}
	static token_t make_null(const std::string& path_, const std::string& name_) {
		return token_t(path_, Type::NULL_TYPE, name_, "NULL", 0, 0);
	}
	// object
	static token_t make_start_object(const std::string& path_, const std::string& name_) {
		return token_t(path_, Type::START_OBJECT, name_, "", 0, 0);
	}
	static token_t make_end_object(const std::string& path_, const std::string& name_) {
		return token_t(path_, Type::END_OBJECT, name_, "", 0, 0);
	}
	// array
	static token_t make_start_array(const std::string& path_, const std::string& name_) {
		return token_t(path_, Type::START_ARRAY, name_, "", 0, 0);
	}
	static token_t make_end_array(const std::string& path_, const std::string& name_) {
		return token_t(path_, Type::END_ARRAY, name_, "", 0, 0);
	}

	// check type
	bool is_item() const {
		switch(m_type) {
		case Type::STRING:
		case Type::BOOL:
		case Type::SIGNED_INTEGER:
		case Type::UNSIGNED_INTEGER:
		case Type::FLOAT:
		case Type::NULL_TYPE:
			return true;
		default:
			return false;
		}
	}
	bool is_string() const {
		return m_type == Type::STRING;
	}
	bool is_bool() const {
		return m_type == Type::BOOL;
	}
	bool is_integer() const {
		return m_type == Type::SIGNED_INTEGER || m_type == Type::UNSIGNED_INTEGER;
	}
	bool is_float() const {
		return m_type == Type::FLOAT;
	}
	bool is_null() const {
		return m_type == Type::NULL_TYPE;
	}
	bool is_start() const {
		return m_type == Type::START_OBJECT || m_type == Type::START_ARRAY;
	}
	bool is_end() const {
		return m_type == Type::END_OBJECT || m_type == Type::END_ARRAY;
	}
	bool is_array() const {
		return m_type == Type::START_ARRAY || m_type == Type::END_ARRAY;
	}
	bool is_object() const {
		return m_type == Type::START_OBJECT || m_type == Type::END_OBJECT;
	}
	bool is_start_object() const {
		return m_type == Type::START_OBJECT;
	}
	bool is_start_array() const {
		return m_type == Type::START_ARRAY;
	}
	bool is_end_object() const {
		return m_type == Type::END_OBJECT;
	}
	bool is_end_array() const {
		return m_type == Type::END_ARRAY;
	}
	// path
	//	void path_name(const std::string& path, const std::string& name);
	void path_name(const std::string& path, const std::string& name) {
		m_path = path;
		m_name = name;
	}
	void path(const std::string& path) {
		m_path = path;
	}

	// return contents
	const std::string& path() const {
		return m_path;
	}
	Type type() const {
		return m_type;
	}
	const std::string& name() const {
		return m_name;
	}
	const std::string& value() const {
		return m_value;
	}
	// return type specific value
	bool boolValue() const;
	template<typename T>
	T intValue() const {
		T t;
		return t;
	}
	template<>
	std::int64_t intValue() const;
	template<>
	std::int32_t intValue() const;
	template<>
	std::uint64_t intValue() const;
	template<>
	std::uint32_t intValue() const;

	double doubleValue() const;

	std::string value_string() const {
		switch(m_type) {
		case Type::BOOL:
			return boolValue() ? "true" : "false";
		case Type::STRING:
			return "\"" + json_quote(m_value) + "\""; // FIXME need quote special character
		case Type::SIGNED_INTEGER:
		case Type::UNSIGNED_INTEGER:
		case Type::FLOAT:
			return m_value;
		case Type::NULL_TYPE:
			return "null";
		case Type::START_OBJECT:
			return "{";
		case Type::END_OBJECT:
			return "}";
		case Type::START_ARRAY:
			return "[";
		case Type::END_ARRAY:
			return "]";
		default:
			assert(false);
			return "*ERROR*";
		}
	}

	const std::string& type_string() const {
		return to_string(m_type);
	}
};


typedef std::vector<token_t> token_list_t;

// utility function for token_list_t
void fix_path_name(token_list_t& list);

// remove empty object and empty array
void normalize(token_list_t& list);

//
// copy object with nest level
//
token_list_t copy_object(const token_list_t::const_iterator begin, const token_list_t::const_iterator end, const int max_nest_level = 0);
inline token_list_t copy_object(const token_list_t& list, const int max_nest_level = 0) {
	if (list.empty()) return list;
	return copy_object(list.cbegin(), list.cend(), max_nest_level);
}


//
// list object by name value
//
std::vector<token_list_t> list_object_by_name_value(const token_list_t& list, const std::string& name, const std::string& value);


//
// get object by name value
//
inline std::optional<token_list_t> get_object_by_name_value(const token_list_t& list, const std::string& name, const std::string& value) {
	auto result_list = list_object_by_name_value(list, name, value);
	int  result_size = result_list.size();

	if (result_size == 0) {
		return std::nullopt;
	} else if (result_size == 1) {
		return result_list.front();
	} else {
		assert(false);
	}
}


//
// list item by name
//
token_list_t list_item_by_name (const token_list_t::const_iterator begin, const token_list_t::const_iterator end, const std::string& name, const int max_nest_level = 0);
inline token_list_t list_item_by_name (const token_list_t& list, const std::string& name, const int max_nest_level = 0) {
	if (list.empty()) return list;
	return list_item_by_name(list.cbegin(), list.cend(), name, max_nest_level);
}

//
// get item by name
//
inline std::optional<token_t> get_item_by_name(const token_list_t& list, const std::string& name, const int max_nest_level = 0) {
	auto result_list = list_item_by_name(list, name, max_nest_level);
	int  result_size = result_list.size();

	if (result_size == 0) {
		return std::nullopt;
	} else if (result_size == 1) {
		return result_list.front();
	} else {
		assert(false);
	}
}

//
// find item by name value
//
inline bool find_item_by_name_value(const token_list_t::const_iterator begin, const token_list_t::const_iterator end,
	const std::string& name, const std::string& value, const int max_nest_level = 0) {
	token_list_t list = list_item_by_name(begin, end, name, max_nest_level);
	for(const token_t& token: list) {
		if (token.value() == value) return true;
	}

	return false;
}




//
// dump
//
void dump(const std::string& prefix, const token_list_t& list);




//
// parse function
//
class handler_t {
public:
	virtual ~handler_t() {}

	// life cycle event
	virtual void start() = 0;
	virtual void stop()  = 0;

	// data event
	virtual void data(const token_t& token) = 0;
};

void parse(std::istream& in, handler_t *handler);

}
