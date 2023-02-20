//
// JSON
//

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "../util/Util.h"

namespace json {


//
// data
//
class token_t {
	enum class Type {
		STRING, BOOL, SIGNED_INTEGER, UNSIGNED_INTEGER, FLOAT, NULL_TYPE,
		START_OBJECT, END_OBJECT,
		START_ARRAY,  END_ARRAY,
	};

	static std::string to_string(Type type) {
		switch(type) {
		case Type::STRING:
			return "STRING";
		case Type::BOOL:
			return "BOOL";
		case Type::SIGNED_INTEGER:
			return "SIGNED_INTEGER";
		case Type::UNSIGNED_INTEGER:
			return "UNSIGNED_INTEGER";
		case Type::FLOAT:
			return "FLOAT";
		case Type::NULL_TYPE:
			return "NULL_TYPE";
		case Type::START_OBJECT:
			return "START_OBJECT";
		case Type::END_OBJECT:
			return "END_OBJECT";
		case Type::START_ARRAY:
			return "START_ARRAY";
		case Type::END_ARRAY:
			return "END_ARRAY";
		default:
			assert(false);
			return "*ERROR*";
		}
	}

	std::string m_path;
	Type        m_type;
	std::string m_name;
	std::string m_value;
	//
	int64_t     m_int64_value;
	double      m_double_value;

	token_t(const std::string& path_, Type type_, const std::string& name_, const std::string& value_, int64_t int64_value_, double double_value_) :
		m_path(path_), m_type(type_), m_name(name_), m_value(value_), m_int64_value(int64_value_), m_double_value(double_value_) {}


public:
	token_t() :
		m_path("*NULL*"),
		m_type(Type::NULL_TYPE),
		m_name("*NULL*"),
		m_value("*NULL*"),
		m_int64_value(0),
		m_double_value(0) {}

	token_t(const token_t& that, const std::string& path_) :
		m_path(path_),
		m_type(that.m_type),
		m_name(that.m_name),
		m_value(that.m_value),
		m_int64_value(that.m_int64_value),
		m_double_value(that.m_double_value) {}
	token_t(token_t&& that, const std::string& path_) noexcept :
		m_path(path_),
		m_type(that.m_type),
		m_name(std::move(that.m_name)),
		m_value(std::move(that.m_value)),
		m_int64_value(that.m_int64_value),
		m_double_value(that.m_double_value) {}

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
	int64_t intValue() const;
	template<>
	int32_t intValue() const;
	template<>
	uint64_t intValue() const;
	template<>
	uint32_t intValue() const;

	double doubleValue() const;

	std::string value_string() const {
		switch(m_type) {
		case Type::BOOL:
			return boolValue() ? "true" : "false";
		case Type::STRING:
			return "\"" + m_value + "\""; // FIXME need quote special character
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

	std::string type_string() const {
		return to_string(m_type);
	}
};


typedef std::vector<const token_t> token_list_t;
typedef std::vector<std::string>   string_list_t;


class handler_t {
public:
	virtual ~handler_t() {}

	// life cycle event
	virtual void start() = 0;
	virtual void stop()  = 0;

	// data event
	virtual void data(const token_t& token) = 0;
};


//
// utility function
//
std::string glob_to_regex(std::string glob);

void dump(const std::string& prefix, const token_list_t& list);
void dump(const std::string& prefix, const token_t&      token);

void dump_item(const std::string& prefix, const token_list_t& list);
void dump_item(const std::string& prefix, const token_t&      token);



//
// parse function
//
void parse(std::istream& in, handler_t *handler);

}
