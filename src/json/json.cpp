//
// json.cpp
//

#include <nlohmann/json.hpp>

#include <regex>
#include <cstdint>


#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");

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
int64_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. INT64_MAX]
		uint64_t value     = (uint64_t)m_int64_value;
		uint64_t min_value = 0;
		uint64_t max_value = INT64_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// SAME TYPE
	} else {
		ERROR();
	}
	return m_int64_value;
}
template<>
int32_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. INT32_MAX]
		uint64_t value     = (uint64_t)m_int64_value;
		uint64_t max_value = INT32_MAX;
		if (!(value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [INT32_MIN .. INT32_MAX]
		int64_t value     = m_int64_value;
		int64_t min_value = INT32_MIN;
		int64_t max_value = INT32_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		ERROR();
	}
	return m_int64_value;
}
template<>
uint64_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// SAME TYPE
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [0 .. INT64_MAX]
		int64_t value     = m_int64_value;
		int64_t min_value = 0;
		int64_t max_value = INT64_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		ERROR();
	}
	return (uint64_t)m_int64_value;
}
template<>
uint32_t token_t::intValue() const {
	// check overflow
	if (m_type == Type::UNSIGNED_INTEGER) {
		// valid range is [0 .. UINT32_MAX]
		uint64_t value     = (uint64_t)m_int64_value;
		uint64_t max_value = UINT32_MAX;
		if (!(value <= max_value)) ERROR();
	} else if (m_type == Type::SIGNED_INTEGER) {
		// valid range is [0 .. INT32_MAX]
		int64_t value     = m_int64_value;
		int64_t min_value = 0;
		int64_t max_value = INT32_MAX;
		if (!(min_value <= value && value <= max_value)) ERROR();
	} else {
		ERROR();
	}
	return (uint64_t)m_int64_value;
}

double token_t::doubleValue() const {
	if (m_type != Type::FLOAT) ERROR();
	return m_double_value;
}



//
// element function
//
void build_array_element (element_t& parent, token_list_t::const_iterator& i, token_list_t::const_iterator end);
void build_object_element(element_t& parent, token_list_t::const_iterator& i, token_list_t::const_iterator end);

void list_to_element(const token_list_t& list, element_t& root) {
	assert(1 <= list.size());

	token_list_t::const_iterator i = list.cbegin();

	root = element_t(*i++);

	if (root.is_item()) {
		assert(i == list.cend());
	} else if (root.is_start_object()) {
		build_object_element(root, i, list.cend());
	} else if (root.is_start_array()) {
		build_array_element(root, i, list.cend());
	} else {
		assert(false);
	}
}
void build_object_element(element_t& parent, token_list_t::const_iterator& i, token_list_t::const_iterator end) {
	for(;;) {
		if (i == end) break;
		element_t child(*i++);
		if (child.is_item()) {
			parent.add_child(child);
		} else if (child.is_start_object()) {
			build_object_element(child, i, end);
			parent.add_child(child);
		} else if (child.is_start_array()) {
			build_array_element(child, i, end);
			parent.add_child(child);
		} else if (child.is_end_object()) {
			parent.add_child(child);
			break;
		} else if (child.is_end_array()){
			assert(false);
		} else {
			assert(false);
		}
	}
}
void build_array_element(element_t& parent, token_list_t::const_iterator& i, token_list_t::const_iterator end) {
	for(;;) {
		if (i == end) break;
		element_t child(*i++);
		if (child.is_item()) {
			parent.add_child(child);
		} else if (child.is_start_object()) {
			build_object_element(child, i, end);
			parent.add_child(child);
		} else if (child.is_start_array()) {
			build_array_element(child, i, end);
			parent.add_child(child);
		} else if (child.is_end_object()) {
			assert(false);
		} else if (child.is_end_array()){
			parent.add_child(child);
			break;
		} else {
			assert(false);
		}
	}
}

void dump_array(const std::string& prefix, const element_t& element);
void dump_item(const std::string& prefix, const token_t& token) {
	logger.info("%s%s %s", prefix, token.path(), token.value());
}
void dump_item(const std::string& prefix, const element_t& element) {
	dump_item(prefix, element.m_token);
}
void dump_object(const std::string& prefix, const element_t& element) {
	std::string new_prefix = prefix + "    ";
	for(const auto& [key, child] : element.m_object) {
		if (child.is_item()) {
			logger.info("%s%s: %s", new_prefix, key, child.value_string());
		} else if (child.is_start_object()) {
			logger.info("%s%s: {", new_prefix, key);
			dump_object(new_prefix, child);
			logger.info("%s}", new_prefix);
		} else if (child.is_start_array()) {
			logger.info("%s%s: [", new_prefix, key);
			dump_array(new_prefix, child);
			logger.info("%s]", new_prefix);
		} else if (child.is_end_object()) {
			//
		} else {
			logger.error("token %s %s %s", child.path(), child.type_string(), child.value_string());
			assert(false);
		}
	}
}
void dump_array(const std::string& prefix, const element_t& element) {
	std::string new_prefix = prefix + "    ";
	for(const auto& value: element.m_array) {
		const token_t& token = value.m_token;
		if (token.is_item()) {
			logger.info("%s%s", new_prefix, token.value_string());
		} else if (token.is_start_object()) {
			logger.info("%s{", new_prefix);
			dump_object(new_prefix, value);
			logger.info("%s}", new_prefix);
		} else if (token.is_start_array()) {
			logger.info("%s[", new_prefix);
			dump_array(new_prefix, value);
			logger.info("%s]", new_prefix);
		} else if (token.is_end_array()) {
			//
		} else {
			assert(false);
		}
	}
}
void dump(const std::string& prefix, const element_t& element) {
	if (element.is_item()) {
		dump_item(prefix, element);
	} else if (element.is_start_object()) {
		logger.info("%s{", prefix);
		dump_object(prefix, element);
		logger.info("%s}", prefix);
	} else if (element.is_start_array()) {
		logger.info("%s[");
		dump_array(prefix, element);
		logger.info("%s]");
	}
}


void dump(const std::string& prefix, const token_list_t& list) {
	for(const auto& e: list) {
		logger.info("%s%s %s", prefix, e.path(), e.value_string());
	}
}


void element_to_list_object(const element_t& element, token_list_t& list);
void element_to_list_array (const element_t& element, token_list_t& list);
void element_to_list_object(const element_t& element, token_list_t& list) {
	list.emplace_back(element.m_token);
	for(const auto& [key, value] : element.m_object) {
		const element_t& child(value);
		if (child.is_item()) {
			list.emplace_back(child.m_token);
		} else if (child.is_start_object()) {
			element_to_list_object(child, list);
		} else if (child.is_start_array()) {
			element_to_list_array(child, list);
		} else if (child.is_end_object()) {
			list.emplace_back(child.m_token);
		} else {
			logger.error("child %s %s %s", child.path(), child.type_string(), child.value_string());
			assert(false);
		}
	}
}
void element_to_list_array (const element_t& element, token_list_t& list) {
	list.emplace_back(element.m_token);
	for(const auto& value: element.m_array) {
		const element_t& child(value);
		if (child.is_item()) {
			list.emplace_back(child.m_token);
		} else if (child.is_start_object()) {
			element_to_list_object(child, list);
		} else if (child.is_start_array()) {
			element_to_list_array(child, list);
		} else if (child.is_end_array()) {
			list.emplace_back(child.m_token);
		} else {
			logger.error("child %s %s %s", child.path(), child.type_string(), child.value_string());
			assert(false);
		}
	}
}
void element_to_list(const element_t& element, token_list_t& list) {
	if (element.is_item()) {
		list.emplace_back(element.m_token);
	} else if (element.is_start_object()) {
		element_to_list_object(element, list);
	} else if (element.is_start_array()) {
		element_to_list_array(element, list);
	}
}




//
// utility function
//
std::string glob_to_regex(std::string glob) {
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


}
