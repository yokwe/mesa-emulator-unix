//
// json.cpp
//

#include <nlohmann/json.hpp>

#include "json.h"


#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");

namespace json {


//
// utility function
//
std::string glob_to_regex(std::string glob) {
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

bool empty_item(const token_list_t& token_list) {
	for(const token_t& token: token_list) {
		if (token.type == token_t::Type::ITEM) return false;
	}
	return true;
}


//
// parse function
//

class json_token_t : public nlohmann::json::json_sax_t {
public:
	//
	// context
	//
	class context_t {
		bool        m_arrayFlag;
		int         m_arrayIndex;
		std::string m_name;

	public:
		context_t(bool isArray, const std::string& name) :
			m_arrayFlag(isArray),
			m_arrayIndex(0),
			m_name(name) {}
		// copy constructor
		context_t(const context_t& that) :
			m_arrayFlag(that.m_arrayFlag),
			m_arrayIndex(that.m_arrayIndex),
			m_name(that.m_name) {}
		// move constructor
		context_t(context_t&& that) noexcept :
			m_arrayFlag(that.m_arrayFlag),
			m_arrayIndex(that.m_arrayIndex),
			m_name(std::move(that.m_name)) {}


		std::string make_name(const std::string& key) {
			return m_arrayFlag ? std::to_string(m_arrayIndex++) : key;
		}
		bool array() {
			return m_arrayFlag;
		}
		const std::string& name() {
			return m_name;
		}
	};


	handler_t* m_handler;

	json_token_t() : m_handler(nullptr) {}


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


	std::vector<context_t> m_stack;
	std::string make_name(const std::string& key) {
		if (m_stack.empty()) {
			return std::string();
		} else {
			return m_stack.back().make_name(key);
		}
	}
	void push(const context_t& newValue) {
		m_stack.push_back(newValue);
	}
	void pop() {
		m_stack.pop_back();
	}
	int  level() {
		return (int)m_stack.size();
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
	void item(const std::string& key, const std::string& value) {
		std::string name = make_name(key);
		token_t token = token_t::item(name, value);
		m_handler->item(token);
	}
	bool null() override {
		if (true) item(lastKey, "NULL");
		return true;
	}
	bool boolean(bool newValue) override {
		if (true) item(lastKey, (newValue ? "TRUE" : "FALSE"));
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		if (true) item(lastKey, std::to_string(newValue));
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		if (true) item(lastKey, std::to_string(newValue));
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		(void)newValue;
		if (true) item(lastKey, newValueString);
		return true;
	}
	bool string(string_t& newValue) override {
		if (true) item(lastKey, newValue);
		return true;
	}


	//
	// container
	//
	void leave() {
		token_t token = token_t::leave();
		m_handler->leave(token);
		// update m_stack
		pop();
	}
	void enter(const std::string& key, bool isArray) {
		std::string name = make_name(key);
		token_t token = token_t::enter(name, isArray);
		m_handler->enter(token);
		// update m_stack
		context_t my(isArray, name);
		push(my);
	}
	// object
	bool start_object(std::size_t) override {
		bool isArray = false;

		enter(lastKey, isArray);
		return true;
	}
	bool end_object() override {
		leave();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool isArray = true;

		enter(lastKey, isArray);
		return true;
	}
	bool end_array() override {
		leave();
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



void parse(std::istream& in, handler_t *handler) {
	json_token_t json_token;

	json_token.parse(in, handler);
}


void parse(const token_list_t& token_list, handler_t *handler) {
	handler->start();
	for(const token_t& token: token_list) {
		switch(token.type) {
		case token_t::Type::ITEM:
			handler->item(token);
			break;
		case token_t::Type::ENTER:
			handler->enter(token);
			break;
		case token_t::Type::LEAVE:
			handler->leave(token);
			break;
		default:
			logger.error("Unexpected type");
			logger.error("  type %d", token.type);
			ERROR();
		}
	}
	handler->stop();
}


}
