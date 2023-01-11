//
// json.cpp
//

#include <ios>
#include <vector>
#include <regex>

#include "json.h"

#include <nlohmann/json.hpp>

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");


//
// Use unnamed namespace to hide variables and classes
//
namespace {

std::string NULL_STRING  = "NULL";
std::string TRUE_STRING  = "TRUE";
std::string FALSE_STRING = "FALSE";


class value {
public:
	enum Type {
		NULL_, BOOL, INT, UINT, FLOAT, STRING,
	};

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


class json_dump : public nlohmann::json::json_sax_t {
public:
	std::string             lastKey;
	std::vector<container>  stack;
	std::vector<std::regex> pathFilters;

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

	void process(const value& value) {
		if (top().filter()) return;

		std::string path = top().getPath(lastKey);
		if (matchPathFilter(path)) {
			// filtered
		} else {
			std::string line = path + " " + value.to_string();
			std::cout << line << std::endl;
		}
	}

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
	void process(bool inArray) {
		if (empty()) {
			container container("", inArray);
			push(container);
		} else {
			std::string path = top().getPath(lastKey);
			container container(path, inArray);
			if (matchPathFilter(path)) container.setFilter();
			push(container);
		}
	}
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
	// binary ???
	//
	bool binary(nlohmann::json::binary_t& /*val*/) override {
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

	return nlohmann::json::sax_parse(in, &sax);
}
