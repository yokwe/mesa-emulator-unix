//
// json.cpp
//

#include <nlohmann/json.hpp>

#include <regex>


#include "../util/Util.h"
static const util::Logger logger(__FILE__);

#include "json.h"


namespace json {


//
// token_t
//
bool token_t::boolValue() const {
	if (m_type != Type::BOOL) ERROR();
	return m_int64_value != 0;
}
template<>
std::int64_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. INT64_MAX]
		std::uint64_t value     = (uint64_t)m_int64_value;
		std::uint64_t min_value = 0;
		std::uint64_t max_value = INT64_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// SAME TYPE
	} else {
		ERROR();
	}
	return m_int64_value;
}
template<>
std::int32_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. INT32_MAX]
		std::uint64_t value     = (uint64_t)m_int64_value;
		std::uint64_t max_value = INT32_MAX;
		if (!(value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [INT32_MIN .. INT32_MAX]
		std::int64_t value     = m_int64_value;
		std::int64_t min_value = INT32_MIN;
		std::int64_t max_value = INT32_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		logger.info("m_type %s %s", name(), type_string());
		ERROR();
	}
	return (std::int32_t)m_int64_value;
}
template<>
std::uint64_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// SAME TYPE
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [0 .. INT64_MAX]
		std::int64_t value     = m_int64_value;
		std::int64_t min_value = 0;
		std::int64_t max_value = INT64_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		ERROR();
	}
	return (uint64_t)m_int64_value;
}
template<>
std::uint32_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. UINT32_MAX]
		std::uint64_t value     = (uint64_t)m_int64_value;
		std::uint64_t max_value = UINT32_MAX;
		if (!(value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [0 .. INT32_MAX]
		std::int64_t value     = m_int64_value;
		std::int64_t min_value = 0;
		std::int64_t max_value = INT32_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		ERROR();
	}
	return (std::uint64_t)m_int64_value;
}

double token_t::doubleValue() const {
	if (m_type != Type::FLOAT) ERROR();
	return m_double_value;
}


//
// utility function for token_list_t
//
void dump(const std::string& prefix, const token_list_t& list) {
	for(const auto& e: list) {
		logger.info("%s%s %s", prefix, e.path(), e.value_string());
	}
}


//
// utility function
//
std::string glob_to_regex_string(const std::string& glob) {
	// special case **/abc/** => .*?/abc(?:/.*?)?
	{
		std::regex regex("\\*\\*/([^/]+)/\\*\\*");
		std::smatch match;
		if (std::regex_match(glob, match, regex)) {
			return ".*?/" + match[1].str() + "(?:/.*?)?";
		}
	}

	// *  => [^/]*?
	// ** => .*?
	std::string ret;
	for(size_t i = 0; i < glob.size(); i++) {
		char c = glob[i];
		switch(c) {
		case '?':
			ret += "[^/]";
			break;
		case '*':
			if ((i + 1) == glob.size()) {
				ret += "[^/]*?"; // *
			} else {
				char next = glob[i + 1];
				if (next == '*') {
					ret += ".*?"; // **
					i++;
				} else {
					ret += "[^/]*?"; // *
				}
			}
			break;
		case '+':
		case '^':
		case '$':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '|':
		case '\\':
			ret += '\\';
			ret += c;
			break;
		default:
			ret += c;
			break;
		}
	}
	return ret;
}


std::regex glob_to_regex(const std::string& glob) {
	return std::regex(glob_to_regex_string(glob));
}
std::regex  glob_to_regex(const std::initializer_list<std::string>& list) {
	std::string string;
	for (const std::string& e : list) {
		string.append("|(?:" + glob_to_regex_string(e) + ")");
	}
	return std::regex(string);
}

std::string json_quote(const std::string& string) {
	std::string ret;

	for(size_t i = 0; i < string.size(); i++) {
		char c = string[i];
		switch(c) {
		case '\"':
			ret += R"(\")";
			break;
		case '\\':
			ret += R"(\\)";
			break;
		case '\b':
			ret += R"(\b)";
			break;
		case '\f':
			ret += R"(\f)";
			break;
		case '\n':
			ret += R"(\n)";
			break;
		case '\r':
			ret += R"(\r)";
			break;
		case '\t':
			ret += R"(\t)";
			break;
		default:
			ret += c;
			break;
		}
	}

	return ret;
}



//
// parse function
//

namespace {

class json_token_t : public nlohmann::json::json_sax_t {
public:
	handler_t* m_handler = nullptr;

	//
	// parse input stream
	//
	void parse(std::istream& in, handler_t* handler) {
		m_handler = handler;

		m_handler->start();
		nlohmann::json::sax_parse(in, this);
		m_handler->stop();

		m_handler = nullptr;
	}



	//
	// context
	//
	class context_t {
		bool        m_arrayFlag;
		int         m_arrayIndex;
		std::string m_path;
		std::string m_name;

	public:
		context_t(bool isArray, const std::string& path_, const std::string& name_) :
			m_arrayFlag(isArray),
			m_arrayIndex(0),
			m_path(path_),
			m_name(name_) {}
		// copy constructor
		context_t(const context_t& that) :
			m_arrayFlag(that.m_arrayFlag),
			m_arrayIndex(that.m_arrayIndex),
			m_path(that.m_path),
			m_name(that.m_name) {}
		// move constructor
		context_t(context_t&& that) noexcept :
			m_arrayFlag(that.m_arrayFlag),
			m_arrayIndex(that.m_arrayIndex),
			m_path(std::move(that.m_path)),
			m_name(std::move(that.m_name)) {}


		std::string make_name(const std::string& key) {
			return m_arrayFlag ? std::to_string(m_arrayIndex++) : key;
		}
		const std::string& path() const {
			return m_path;
		}
		const std::string& name() const {
			return m_name;
		}
	};


	std::vector<context_t> m_stack;
	std::tuple<std::string, std::string> make_path_name(const std::string& key) {
	//         path         name
		if (m_stack.empty()) {
			std::string empty;
			return {empty, empty};
		} else {
			context_t& top = m_stack.back();

			std::string name = top.make_name(key);
			std::string path = top.path() + "/" + name;
			return {path, name};
		}
	}

	//
	// key
	//
	std::string lastKey;
	bool key(string_t& newValue) override {
		lastKey = newValue;
		return true;
	}


	//
	// value
	//
	bool null() override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_null(path, name);
		m_handler->data(token);
		return true;
	}
	bool boolean(bool newValue) override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_bool(path, name, newValue);
		m_handler->data(token);
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_singned_integer(path, name, newValue);
		m_handler->data(token);
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_unsigned_integer(path, name, newValue);
		m_handler->data(token);
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newStringValue) override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_float(path, name, newStringValue, newValue);
		m_handler->data(token);
		return true;
	}
	bool string(string_t& newValue) override {
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_string(path, name, newValue);
		m_handler->data(token);
		return true;
	}


	//
	// container
	//
	// object
	bool start_object(std::size_t) override {
		bool isArray = false;
		//
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_start_object(path, name);
		m_handler->data(token);
		// update m_stack
		context_t my(isArray, path, name);
		m_stack.push_back(my);
		return true;
	}
	bool end_object() override {
		const context_t& top = m_stack.back();
		token_t token = token_t::make_end_object(top.path(), top.name());
		m_handler->data(token);
		// update m_stack
		m_stack.pop_back();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool isArray = true;
		//
		auto [path, name] = make_path_name(lastKey);
		token_t token = token_t::make_start_array(path, name);
		m_handler->data(token);
		// update m_stack
		context_t my(isArray, path, name);
		m_stack.push_back(my);
		return true;
	}
	bool end_array() override {
		const context_t& top = m_stack.back();
		token_t token = token_t::make_end_array(top.path(), top.name());
		m_handler->data(token);
		// update m_stack
		m_stack.pop_back();
		return true;
	}


	//
	// binary
	//
	bool binary(nlohmann::json::binary_t& val) override{
		(void)val;
		ERROR();
		return false;
	}


	//
	// error
	//
	bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override {
		logger.error("parser_error");
		logger.error("  position = %d  last token = %s  ex = %s", (int)position, last_token, ex.what());
		ERROR();
		return false;
	}
};

}


void parse(std::istream& in, handler_t *handler) {
	json_token_t json_token;

	json_token.parse(in, handler);
}



//
// token_list_t
//
struct set_path_name_t {
	struct context_t {
		bool        m_arrayFlag;
		std::string m_path;
		std::string m_name;
		int         m_arrayIndex = 0;

		context_t(bool isArray, const std::string& path, const std::string& name) :
			m_arrayFlag(isArray),
			m_path(path),
			m_name(name) {}

		std::string make_name(const std::string& key) {
			return m_arrayFlag ? std::to_string(m_arrayIndex++) : key;
		}
		std::pair<std::string, std::string> path_name() const {
			return {m_path, m_name};
		}
		std::string path() {
			return m_path;
		}
	};

	std::vector<context_t> m_stack;
	std::pair<std::string, std::string> make_path_name(const std::string& key) {
	//         path         name
		if (m_stack.empty()) {
			std::string empty;
			return {empty, empty};
		} else {
			context_t& top = m_stack.back();
			//
			std::string name = top.make_name(key);
			std::string path = top.path() + "/" + name;
			return {path, name};
		}
	}

	void set(std::vector<token_t>& list) {
		for(token_t& token: list) {
			if (token.is_item()) {
				if (m_stack.empty()) {
					ERROR();
				}
				token.path_name(make_path_name(token.name()));
			} else if (token.is_start()) {
				token.path_name(make_path_name(token.name()));
				// update m_stack
				m_stack.emplace_back(token.is_array(), token.path(), token.name());
			} else if (token.is_end()) {
				const context_t& top = m_stack.back();
				token.path_name(top.path_name());
				// update m_stack
				m_stack.pop_back();
			} else {
				assert(false);
			}
		}
	}
};

void token_list_t::set_path_name() {
	set_path_name_t set_path_name;
	set_path_name.set(m_list);
}

void token_list_t::remove_empty_object_array() {
	for(auto i = m_list.begin(); i != m_list.end(); i++) {
		if (i->is_start_object() && (i + 1)->is_end_object()) {
			i = m_list.erase(i, i + 2); // erase element [i, i + 2)
		}
		if (i->is_start_array() && (i + 1)->is_end_array()) {
			i = m_list.erase(i, i + 2); // erase element [i, i + 2)
		}
	}
}


}
