//
//
//

#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>


namespace json {
namespace impl {
//
// json_value
//
class json_value {
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
	json_value():
		type(Type::NULL_), string(NULL_STRING)  {}
	json_value(bool newValue) :
		type(Type::BOOL), string(newValue ? TRUE_STRING : FALSE_STRING) {}
	json_value(int64_t newValue) :
		type(Type::INT), string(std::to_string(newValue)) {}
	json_value(uint64_t newValue) :
		type(Type::UINT), string(std::to_string(newValue)) {}
	json_value(double /*newValue*/, std::string newValueString) :
		type(Type::FLOAT), string(newValueString) {}
	json_value(std::string newValue) :
		type(Type::STRING), string(newValue) {}

	// copy constructor
	json_value(const json_value& that) :
		type(that.type),
		string(that.string) {}

	// move constructor
	json_value(json_value&& that) noexcept :
		type(that.type),
		string(std::move(that.string)) {}

	Type to_type() const {
		return type;
	}
	const std::string& to_string() const {
		return string;
	}
};

//
// json_container
//
class json_container {
	std::string path;
	bool        isArray; // array or record
	int         arrayIndex;
	bool        filterFlag; // true => filter  false => not filtered

public:
	void setFilter() {
		filterFlag = true;
	}
	void clearFilter() {
		filterFlag = false;
	}
	bool filter() const {
		return filterFlag;
	}

	std::string getName(const std::string& key) {
		return isArray ? std::to_string(arrayIndex++) : key;
	}
	const std::string& getPath() const {
		return path;
	}

	// constructor
	json_container() :
		path(""), isArray(false), arrayIndex(0), filterFlag(false) {}
	json_container(std::string path_, bool isArray_) :
		path(path_), isArray(isArray_), arrayIndex(0), filterFlag(false) {}

	// copy constructor
	json_container(const json_container& that) :
		path(that.path), isArray(that.isArray), arrayIndex(that.arrayIndex), filterFlag(that.filterFlag) {}

	// move constructor
	json_container(json_container&& that) noexcept :
		path(std::move(that.path)), isArray(that.isArray), arrayIndex(that.arrayIndex), filterFlag(that.filterFlag) {}
};


//
// json_sax base class of json sax parser
//
class json_sax : public nlohmann::json::json_sax_t {
public:
	std::string                 lastKey;
	std::vector<json_container> stack;
	int                         countValue;

	int parse(std::istream& in) {
		countValue = 0;
		bool result = nlohmann::json::sax_parse(in, this);
		if (!result) countValue = -countValue;
		return countValue;
	}

	bool empty() {
		return stack.empty();
	}
	void push(const json_container& newValue) {
		stack.push_back(newValue);
	}
	void pop() {
		stack.pop_back();
	}
	json_container& top() {
		return stack.back();
	}


	virtual bool process(const std::string& path, const std::string& name, const json_value& value) = 0;

	void processCount(const json_value& value) {
		json_container& parent = top();

		std::string name = parent.getName(lastKey);
		std::string path = parent.getPath() + "/" + name;

		if (process(path, name, value)) countValue++;
	}

	bool null() override {
		json_value value;

		processCount(value);
		return true;
	}
	bool boolean(bool newValue) override {
		json_value value(newValue);

		processCount(value);
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		json_value value(newValue);

		processCount(value);
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		json_value value(newValue);

		processCount(value);
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		json_value value(newValue, newValueString);

		processCount(value);
		return true;
	}
	bool string(string_t& newValue) override {
		json_value value(newValue);

		processCount(value);
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
	virtual void process(const std::string& path, const std::string& name, json_container& conatiner) = 0;

	void process(bool isArray) {
		std::string path;
		std::string name;

		if (empty()) {
			// Special case for root
		} else {
			json_container& parent = top();

			name = parent.getName(lastKey);
			path = parent.getPath() + "/" + name;
		}

		json_container container(path, isArray);
		process(path, name, container);
		push(container);
	}
	// object
	bool start_object(std::size_t) override {
		bool isArray = false;

		process(isArray);
		return true;
	}
	bool end_object() override {
		pop();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool isArray = true;

		process(isArray);
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
