//
// JSON
//

#pragma once

#include <iostream>
#include <ios>
#include <vector>

#include <nlohmann/json.hpp>

namespace json {


class value {
public:
	enum Type {
		NULL_, BOOL, INT, UINT, FLOAT, STRING,
	};

	static const std::string NULL_STRING;
	static const std::string TRUE_STRING;
	static const std::string FALSE_STRING;

	const Type        type;
	const std::string string;

	// constructor
	value():
		type(Type::NULL_), string(NULL_STRING)  {}
	value(bool newValue) :
		type(Type::BOOL), string(newValue ? TRUE_STRING : FALSE_STRING) {}
	value(int64_t newValue) :
		type(Type::INT), string(std::to_string(newValue)) {}
	value(uint64_t newValue) :
		type(Type::UINT), string(std::to_string(newValue)) {}
	value(double /*newValue*/, std::string newValueString) :
		type(Type::FLOAT), string(newValueString) {}
	value(std::string newValue) :
		type(Type::STRING), string(newValue) {}

	// copy constructor
	value(const value& that) :
		type(that.type),
		string(that.string) {}

	// move constructor
	value(value&& that) noexcept :
		type(that.type),
		string(std::move(that.string)) {}

	Type to_type() const {
		return type;
	}
	const std::string& to_string() const {
		return string;
	}
};


class container {
	std::string path;
	bool        isArray; // array or record
	int         arrayIndex;
	bool        filterFlag; // true => filter  false => not filtered

public:
	std::string getPath(std::string& key) {
		return path + "/" + (isArray ? std::to_string(arrayIndex++) : key);
	}
	void setFilter() {
		filterFlag = true;
	}
	void clearFilter() {
		filterFlag = false;
	}
	bool filter() {
		return filterFlag;
	}

	// default constructor
	container() :
		path(""), isArray(false), arrayIndex(0), filterFlag(false) {}
	container(std::string path_, bool inArray_) :
		path(path_), isArray(inArray_), arrayIndex(0), filterFlag(false) {}

	// copy constructor
	container(const container& that) :
		path(that.path), isArray(that.isArray), arrayIndex(that.arrayIndex), filterFlag(that.filterFlag) {}

	// move constructor
	container(container&& that) noexcept :
		path(std::move(that.path)), isArray(that.isArray), arrayIndex(that.arrayIndex), filterFlag(that.filterFlag) {}
};


class json_sax : public nlohmann::json::json_sax_t {
public:
	std::string             lastKey;
	std::vector<container>  stack;

	bool empty() {
		return stack.empty();
	}
	void push(const container& newValue) {
		stack.push_back(newValue);
	}
	void pop() {
		stack.pop_back();
	}
	container& top() {
		return stack.back();
	}


	virtual void process(const value& value) = 0;

	bool null() override {
		value value;

		process(value);
		return true;
	}
	bool boolean(bool newValue) override {
		value value(newValue);

		process(value);
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		value value(newValue);

		process(value);
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		value value(newValue);

		process(value);
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		value value(newValue, newValueString);

		process(value);
		return true;
	}
	bool string(string_t& newValue) override {
		value value(newValue);

		process(value);
		return true;
	}

	//
	// key
	//
	bool key(string_t& newValue) override {
		lastKey = newValue;
		return true;
	}

	//
	// container
	//
	virtual void process(bool inArray) = 0;

	// object
	bool start_object(std::size_t) override {
		bool inArray = false;

		process(inArray);
		return true;
	}
	bool end_object() override {
		pop();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool inArray = true;

		process(inArray);
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


//
// utility
//
std::string glob_to_regex(std::string glob);

//
//
//
bool dump(std::istream& in);

}
