//
// json.cpp
//

#include <vector>
#include <regex>

#include <nlohmann/json.hpp>

#include "json.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");


// to hide detail, use unnamed namespace
namespace {

//
// static variable definition
//

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
const std::string json_value::NULL_STRING  = "NULL";
const std::string json_value::TRUE_STRING  = "TRUE";
const std::string json_value::FALSE_STRING = "FALSE";


//
// json_container
//
class json_container {
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
	json_container() :
		path(""), isArray(false), arrayIndex(0), filterFlag(false) {}
	json_container(std::string path_, bool inArray_) :
		path(path_), isArray(inArray_), arrayIndex(0), filterFlag(false) {}

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

	bool parse(std::istream& in) {
		return nlohmann::json::sax_parse(in, this);
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


	virtual void process(const json_value& value) = 0;

	bool null() override {
		json_value value;

		process(value);
		return true;
	}
	bool boolean(bool newValue) override {
		json_value value(newValue);

		process(value);
		return true;
	}
	bool number_integer(number_integer_t newValue) override {
		json_value value(newValue);

		process(value);
		return true;
	}
	bool number_unsigned(number_unsigned_t newValue) override {
		json_value value(newValue);

		process(value);
		return true;
	}
	bool number_float(number_float_t newValue, const string_t& newValueString) override {
		json_value value(newValue, newValueString);

		process(value);
		return true;
	}
	bool string(string_t& newValue) override {
		json_value value(newValue);

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
	bool binary(nlohmann::json::binary_t& /*val*/) override {
		ERROR();
		return true;
	}


	//
	// error
	//
	bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override {
		logger.error("parser_error");
		logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
		ERROR();
		return false;
	}
};


//
// json_dum dump dot form json with filter
//
class json_dump : public json_sax {
public:
	std::vector<std::regex> pathFilters;

	void addPathFilter(std::string glob) {
		std::string regex = glob_to_regex(glob);
		std::regex re(regex);
		pathFilters.push_back(re);
	}
	bool matchPathFilter(std::string& path) {
		for(auto re: pathFilters) {
			if (std::regex_match(path, re)) return true;
		}
		return false;
	}

	//
	// value
	//
	void process(const json_value& value) {
		if (top().filter()) return;

		std::string path = top().getPath(lastKey);
		if (matchPathFilter(path)) {
			// filtered
		} else {
			std::string line = path + " " + value.to_string();
			std::cout << line << std::endl;
		}
	}

	//
	// container
	//
	void process(bool inArray) {
		if (empty()) {
			json_container container("", inArray);
			push(container);
		} else {
			std::string path = top().getPath(lastKey);
			json_container container(path, inArray);
			if (matchPathFilter(path)) container.setFilter();
			push(container);
		}
	}
};

}


bool json::dump(std::istream& in) {
	json_dump sax;

	sax.addPathFilter("**Id");
	sax.addPathFilter("**/range/**");
	sax.addPathFilter("**/is*");
	sax.addPathFilter("**/loc/includedFrom/**");
	sax.addPathFilter("**/loc/offset");
	sax.addPathFilter("**/loc/line");
	sax.addPathFilter("**/loc/col");
	sax.addPathFilter("**/loc/tokLen");
	sax.addPathFilter("**/loc/spellingLoc/**");
	sax.addPathFilter("**/loc/expansionLoc/**");
	sax.addPathFilter("**/definitionData/**");
	sax.addPathFilter("**/bases/**");

	return sax.parse(in);
}
