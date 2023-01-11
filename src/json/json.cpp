//
// json.cpp
//

#include "json.h"

#include <nlohmann/json.hpp>

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");


//
// static variables
//
std::string json::value::NULL_STRING  = "NULL";
std::string json::value::TRUE_STRING  = "TRUE";
std::string json::value::FALSE_STRING = "FALSE";



class json_dump : public nlohmann::json::json_sax_t {
public:
	std::string                  lastKey;
	std::vector<json::container> stack;

	bool empty() {
		return stack.empty();
	}
	void push(const json::container& newValue) {
		stack.push_back(newValue);
	}
	void pop() {
		stack.pop_back();
	}
	json::container& top() {
		return stack.back();
	}


	std::string toLine(const std::string& value) {
	   	json::container& current = top();
		std::string      name    = current.inArray ? current.getIndexName() : lastKey;
		std::string      ret     = std_sprintf("%s/%s %s", current.path, name, value);
		return ret;
	}

	bool null() override {
		json::value value;
		std::string line = toLine(value.to_string());
		std::cout << line << std::endl;
		return true;
	}
	bool boolean(bool newValue) override {
		json::value value(newValue);
		std::string line = toLine(value.to_string());
		std::cout << line << std::endl;
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		json::value value(newValue);
		std::string line = toLine(value.to_string());
		std::cout << line << std::endl;
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		json::value value(newValue);
		std::string line = toLine(value.to_string());
		std::cout << line << std::endl;
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		json::value value(newValue, newValueString);
		std::string line = toLine(value.to_string());
		std::cout << line << std::endl;
		return true;
	}
	bool string(string_t& newValue) override {
		json::value value(newValue);
		std::string line = toLine(newValue);
		std::cout << line << std::endl;
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
	void container(bool inArray) {
		if (empty()) {
			json::container container = json::container("", inArray, 0);
			push(container);
		} else {
			json::container& current   = top();
			std::string      name      = current.inArray ? current.getIndexName() : lastKey;
			json::container  container = json::container(current.path + "/" + name, inArray, 0);
			push(container);
		}
	}
	// object
	bool start_object(std::size_t) override {
		bool inArray = false;
		container(inArray);
		return true;
	}
	bool end_object() override {
		pop();
		return true;
	}
	// array
	bool start_array(std::size_t) override {
		bool inArray = true;
		container(inArray);
		return true;
	}
	bool end_array() override {
		pop();
		return true;
	}


	//
	// binary ???
	//
	bool binary(nlohmann::json::binary_t& /*val*/) override {
		DEBUG_TRACE();
		ERROR();
		return true;
	}

	bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override {
		logger.error("parser_error");
		logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
		ERROR();
		return false;
	}
};

bool json::dump(std::istream& in) {
	json_dump sax;
	return nlohmann::json::sax_parse(in, &sax);
}
