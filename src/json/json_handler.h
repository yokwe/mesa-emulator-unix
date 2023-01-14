//
// json_handler.h
//

#pragma once

#include <string>
#include <iostream>

#include <nlohmann/json.hpp>

#include "handler.h"

namespace json {

class json_handler_t : public nlohmann::json::json_sax_t {
public:
	std::string lastKey;
	handler_t*  handler;

	json_handler_t(handler_t* handler_) : handler(handler_) {}


	//
	// parse input stream
	//
	void parse(std::istream& in) {
		handler->start();
		nlohmann::json::sax_parse(in, this);
		handler->stop();
	}


	//
	// key
	//
	bool key(string_t& newValue) override {
		lastKey = newValue;
		return true;
	}


	//
	// value
	//
	void item(const std::string& key, const std::string& value) {
		handler->item(key, value);
	}
	bool null() override {
		item(lastKey, "NULL");
		return true;
	}
	bool boolean(bool newValue) override {
		item(lastKey, newValue ? "TRUE" : "FALSE");
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		item(lastKey, std::to_string(newValue));
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		item(lastKey, std::to_string(newValue));
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		(void)newValue;
		item(lastKey, newValueString);
		return true;
	}
	bool string(string_t& newValue) override {
		item(lastKey, newValue);
		return true;
	}


	//
	// container
	//
	void leave() {
		handler->leave();
	}
	void enter(const std::string& key, bool isArray) {
		handler->enter(key, isArray);
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
	bool binary(nlohmann::json::binary_t& /*val*/) override;


	//
	// error
	//
	bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override;
};


}
