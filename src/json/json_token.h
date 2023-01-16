//
// json_token.h
//

#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../util/Util.h"


namespace json {
namespace token {


class token_t {
public:
	enum Type {
		ITEM, ENTER, LEAVE, END
	};

	const Type        type;
	const std::string name;
	const bool        arrayFlag;
	const std::string value;

	// copy constructor
	token_t(const token_t& that) :
		type(that.type), name(that.name),
		arrayFlag(that.arrayFlag), value(that.value) {}
	// move constructor
	token_t(token_t&& that) noexcept :
		type(that.type), name(std::move(that.name)),
		arrayFlag(that.arrayFlag), value(std::move(that.value)) {}

	static token_t item(const std::string& name, const std::string& value) {
		return token_t(Type::ITEM, name, value);
	}
	static token_t enter(const std::string& name, bool isArray) {
		return token_t(Type::ENTER, name, isArray);
	}
	static token_t leave() {
		static token_t ret(Type::LEAVE);
		return ret;
	}
	static token_t end() {
		static token_t ret(Type::END);
		return ret;
	}

private:
	token_t(Type type_, const std::string& name_, const std::string& value_) :
		type(type_), name(name_), arrayFlag(false), value(value_) {}
	token_t(Type type_, const std::string& name_, bool isArray) :
		type(type_), name(name_), arrayFlag(isArray), value() {}
	token_t( Type type_) :
		type(type_), name(), arrayFlag(false), value() {}
};


typedef std::vector<token_t>     token_list_t;
typedef std::vector<std::string> string_list_t;


class token_handler_t {
public:
	virtual ~token_handler_t() {}

	// life cycle event
	virtual void start() = 0;
	virtual void stop()  = 0;
	// data event
	virtual void item (const token_t& token) = 0;
	virtual void enter(const token_t& token) = 0;
	virtual void leave(const token_t& token) = 0;
};


class token_record_t {
	std::string  m_path;
	int          m_item_count;
	token_list_t m_token_list;
public:
	void start(const std::string& path) {
		m_path       = path;
		m_item_count = 0;
		m_token_list.clear();
	}

	void add(const token_t& token) {
		m_token_list.push_back(token);
		if (token.type == token_t::Type::ITEM) m_item_count++;
	}

	bool empty() {
		return m_item_count == 0;
	}

	const std::string& path() const {
		return m_path;
	}
	int item_count() const {
		return m_item_count;
	}
	const token_list_t& token_list() const {
		return m_token_list;
	}

	void parse(token_handler_t& handler) {
		handler.start();
		for(token_t& token: m_token_list) {
			switch(token.type) {
			case token_t::Type::ITEM:
				handler.item(token);
				break;
			case token_t::Type::ENTER:
				handler.enter(token);
				break;
			case token_t::Type::LEAVE:
				handler.leave(token);
				break;
			default:
				assert(false);
			}
		}
		handler.stop();
	}
};


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


	token_handler_t& m_handler;
	bool             m_proceed;

	json_token_t(token_handler_t& handler_) : m_handler(handler_), m_proceed(true) {}


	//
	// stop parse
	//
	void stop_parse() {
		m_proceed = false;
	}


	//
	// parse input stream
	//
	void parse(std::istream& in) {
		m_handler.start();
		nlohmann::json::sax_parse(in, this);
		m_handler.stop();
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
		return m_proceed;
	}


	//
	// value
	//
	void item(const std::string& key, const std::string& value) {
		std::string name = make_name(key);
		token_t token = token_t::item(name, value);
		m_handler.item(token);
	}
	bool null() override {
		if (m_proceed) item(lastKey, "NULL");
		return m_proceed;
	}
	bool boolean(bool newValue) override {
		if (m_proceed) item(lastKey, (newValue ? "TRUE" : "FALSE"));
		return m_proceed;
	}
	bool number_integer(number_integer_t newValue) override {
		if (m_proceed) item(lastKey, std::to_string(newValue));
		return m_proceed;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		if (m_proceed) item(lastKey, std::to_string(newValue));
		return m_proceed;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		(void)newValue;
		if (m_proceed) item(lastKey, newValueString);
		return m_proceed;
	}
	bool string(string_t& newValue) override {
		if (m_proceed) item(lastKey, newValue);
		return m_proceed;
	}


	//
	// container
	//
	void leave() {
		token_t token = token_t::leave();
		m_handler.leave(token);
		// update m_stack
		pop();
	}
	void enter(const std::string& key, bool isArray) {
		std::string name = make_name(key);
		token_t token = token_t::enter(name, isArray);
		m_handler.enter(token);
		// update m_stack
		context_t my(isArray, name);
		push(my);
	}
	// object
	bool start_object(std::size_t) override {
		bool isArray = false;

		enter(lastKey, isArray);
		return m_proceed;
	}
	bool end_object() override {
		leave();
		return m_proceed;
	}
	// array
	bool start_array(std::size_t) override {
		bool isArray = true;

		enter(lastKey, isArray);
		return true;
	}
	bool end_array() override {
		leave();
		return m_proceed;
	}


	//
	// binary
	//
	bool binary(nlohmann::json::binary_t& val) override;


	//
	// error
	//
	bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override;
};


}
}
