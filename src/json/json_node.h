//
//
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include <nlohmann/json.hpp>

#include "json_impl.h"

namespace json {
namespace node {


class json_node : public nlohmann::json::json_sax_t {
	std::string lastKey;

public:
	class context {
		static std::vector<context> stack;

		bool        isArray;
		int         arrayIndex;
		int         level;
		std::string path;
		std::string name;

		std::string makeName(const std::string& key) {
			return isArray ? std::to_string(arrayIndex++) : key;
		}

	public:
		context(bool isArray_, const std::string& path_, const std::string& name_) :
			isArray(isArray_),
			arrayIndex(0),
			level((int)stack.size()),
			path(path_),
			name(name_) {}

		// copy constructor
		context(const context& that) :
			isArray(that.isArray),
			arrayIndex(that.arrayIndex),
			level(that.level),
			path(that.path),
			name(that.name) {}
		// move constructor
		context(context&& that) noexcept :
			isArray(that.isArray),
			arrayIndex(that.arrayIndex),
			level(that.level),
			path(std::move(that.path)),
			name(std::move(that.name)) {}


		static void push(const context& newValue) {
			stack.push_back(newValue);
		}
		static void pop() {
			stack.pop_back();
		}
		static std::tuple<std::string, std::string> getPathName(const std::string& key) {
			//           path         name
			if (stack.empty()) {
				return {"", ""};
			} else {
				context&    parent = stack.back();
				std::string name   = parent.makeName(key);
				std::string path   = parent.path + "/" + name;
				return {path, name};
			}
		}
	};

	static void push(const context& newValue) {
		context::push(newValue);
	}
	static void pop() {
		context::pop();
	}


	//
	// parse input stream
	//
	bool parse(std::istream& in) {
		return nlohmann::json::sax_parse(in, this);
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
	void processValue(const std::string& value) {
		const auto[path, name] = context::getPathName(lastKey);
		std::cout << path << " " << value << std::endl;
	}
	bool null() override {
		processValue("NULL");
		return true;
	}
	bool boolean(bool newValue) override {
		processValue(newValue ? "TRUE" : "FALSE");
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		processValue(std::to_string(newValue));
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		processValue(std::to_string(newValue));
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		(void)newValue;
		processValue(newValueString);
		return true;
	}
	bool string(string_t& newValue) override {
		processValue(newValue);
		return true;
	}


	//
	// container
	//
	void processContainer(bool isArray) {
		const auto[path, name] = context::getPathName(lastKey);
		context my(isArray, path, name);
		push(my);
	}
	// object
	bool start_object(std::size_t) override {
		bool isArray = false;

		processContainer(isArray);
		return true;
	}
	bool end_object() override {
		pop();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool isArray = true;

		processContainer(isArray);
		return true;
	}
	bool end_array() override {
		pop();
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


int dump(std::istream& in);

}
}
