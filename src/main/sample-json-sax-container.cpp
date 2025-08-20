/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <nlohmann/json.hpp>

#include <iostream>
#include <ios>
#include <limits>
#include <vector>
#include <utility>

using json = nlohmann::json;

#include "../util/Util.h"
static const util::Logger logger(__FILE__);


class sax_value {
private:
	enum Type {
		NULL_, BOOL, INT, UINT, FLOAT, STRING,
	};

	const Type        type;
	const bool        boolValue;
	const int64_t     intValue;
	const uint64_t    uintValue;
	const double      doubleValue;
	const std::string stringValue;

public:
	static const std::string& nullString() {
		static std::string stringNull = "NULL";
		return stringNull;
	}

	static const std::string& to_string(bool value) {
		static std::string stringTrue  = "TRUE";
		static std::string stringFalse = "FALSE";
		return value ? stringTrue : stringFalse;
	}

	sax_value():
		type(Type::NULL_), boolValue(false), intValue(0), uintValue(0), doubleValue(0), stringValue(nullString()) {}
	sax_value(bool newValue) :
		type(Type::BOOL), boolValue(newValue), intValue(0), uintValue(0), doubleValue(0), stringValue(to_string(newValue)) {}
	sax_value(int64_t newValue) :
		type(Type::INT), boolValue(false), intValue(newValue), uintValue((uint64_t)newValue), doubleValue(0), stringValue(std::to_string(newValue)) {}
	sax_value(uint64_t newValue) :
		type(Type::UINT), boolValue(false), intValue((int64_t)newValue), uintValue(newValue), doubleValue(0), stringValue(std::to_string(newValue)) {}
	sax_value(double newValue, std::string newValueString) :
		type(Type::FLOAT), boolValue(false), intValue(0), uintValue(0), doubleValue(newValue), stringValue(newValueString) {}
	sax_value(std::string newValue) :
		type(Type::STRING), boolValue(false), intValue(0), uintValue(0), doubleValue(0), stringValue(newValue) {}

	// copy constructor
	sax_value(const sax_value& that) :
		type(that.type),
		boolValue(that.boolValue),
		intValue(that.intValue),
		uintValue(that.uintValue),
		doubleValue(that.doubleValue),
		stringValue(that.stringValue) {}

	// move constructor
	sax_value(sax_value&& that) noexcept :
		type(that.type),
		boolValue(that.boolValue),
		intValue(that.intValue),
		uintValue(that.uintValue),
		doubleValue(that.doubleValue),
		stringValue(std::move(that.stringValue)) {}

	bool isNull() const {
		return type == Type::NULL_;
	}
	bool to_bool() const {
		assert(type == Type::BOOL);
		return boolValue;
	}
	int64_t to_int() const {
		switch(type) {
		case Type::INT:
			return intValue;
		case Type::UINT:
			if (0 <= intValue) {
				return intValue;
			} else {
				logger.error("out of range");
				ERROR();
			}
			break;
		default:
			logger.error("Unexpected type %d", type);
			ERROR();
		}
		return 0;
	}
	uint64_t to_uint() const {
		switch(type) {
		case Type::INT:
			if (0 <= intValue) {
				return uintValue;
			} else {
				logger.error("out of range");
				ERROR();
			}
			break;
		case Type::UINT:
			return uintValue;
		default:
			logger.error("Unexpected type %d", type);
			ERROR();
		}
		return 0;
	}
	double to_double() const {
		assert(type == Type::FLOAT);
		return doubleValue;
	}
	const std::string& to_string() const {
		return stringValue;
	}
};

class sax_context {
	static std::vector<sax_context> stack;

public:
	static bool empty() {
		return stack.empty();
	}
	static void push(const sax_context& newValue) {
		stack.push_back(newValue);
	}
	static void pop() {
		stack.pop_back();
	}
	static sax_context& top() {
		return stack.back();
	}

	std::string path;
	bool        inArray; // array or record
	int         arrayIndex;

	std::vector<std::string>                   nameList;
	std::unordered_map<std::string, sax_value> map;

	sax_context() :
		path(""), inArray(false), arrayIndex(0) {}
	sax_context(std::string path_, bool inArray_, int arrayIndex_) :
		path(path_), inArray(inArray_), arrayIndex(arrayIndex_) {}
	sax_context(bool inArray_) :
		path(""), inArray(inArray_), arrayIndex(0) {}
	sax_context(const sax_context& that) :
		path(that.path), inArray(that.inArray), arrayIndex(that.arrayIndex) {}


	std::string getIndexName() {
		return std::to_string(arrayIndex++);
	}
	void addField(const std::string& name, const sax_value& value) {
		nameList.push_back(name);
		map.emplace(name, value);
	}

	const std::string getFieldString(int i) {
		std::string name = nameList[i];
		const sax_value& value = map[name];
		return std_sprintf("%s: %s", name, value.to_string());
	}
	const std::string to_string() {
		std::string string;
		int size = (int)nameList.size();
		if (size == 0) {
			//
		} else {
			string = getFieldString(0);
			for(int i = 1; i < size; i++) {
				string.append(", ");
				string.append(getFieldString(i));
			}
		}
		return std_sprintf("%s {%s}", path, string);
	}
};
std::vector<sax_context> sax_context::stack;


class sax_handler : public json::json_sax_t {
  public:
	static constexpr const char* NULL_STRING  = "NULL";
	static constexpr const char* TRUE_STRING  = "TRUE";
	static constexpr const char* FALSE_STRING = "FALSE";

	static const char* to_string(bool value) {
		return value ? TRUE_STRING : FALSE_STRING;
	}

	std::string lastKey;

    //
    // value
    //
	std::string getPath(sax_context& top) {
		std::string name = top.inArray ? top.getIndexName() : lastKey;
		return top.path + "/" + name;
	}

	void saxValue(sax_value& value) {
	   	sax_context& top = sax_context::top();
	   	top.addField(lastKey, value);

//     	std::cout << getPath(top) << " " << value.to_string() << std::endl;
	}
    bool null() override {
    	sax_value value;
    	saxValue(value);
        return true;
    }
    bool boolean(bool newValue) override {
    	sax_value value(newValue);
    	saxValue(value);
        return true;
    }
    bool number_integer(number_integer_t newValue) override {
    	sax_value value(newValue);
	   	saxValue(value);
		return true;
    }
    bool number_unsigned(number_unsigned_t newValue) override {
    	sax_value value(newValue);
	   	saxValue(value);
		return true;
    }
    bool number_float(number_float_t newValue, const string_t& newValueString) override {
    	sax_value value(newValue, newValueString);
	   	saxValue(value);
		return true;
    }
    bool string(string_t& newValue) override {
    	sax_value value(newValue);
	   	saxValue(value);
		return true;
    }

    //
    // key
    //
    bool key(string_t& newValue) override {
    	lastKey = newValue;
        return true;
    }


    void saxContainer(bool inArray) {
    	if (sax_context::empty()) {
    		sax_context context = sax_context("", inArray, 0);
    		sax_context::push(context);
    	} else {
    	   	sax_context& top     = sax_context::top();
	   		sax_context  context = sax_context(getPath(top), inArray, 0);
	   		sax_context::push(context);
    	}
    }
    //
    // object
    //
    bool start_object(std::size_t) override {
    	bool inArray = false;
    	saxContainer(inArray);
    	return true;
    }
    bool end_object() override {
	   	sax_context& top = sax_context::top();
	   	if (!top.nameList.empty()) {
	    	std::cout << "## " << sax_context::top().to_string() << std::endl;
	   	}

    	sax_context::pop();
    	return true;
    }
    //
    // array
    //
    bool start_array(std::size_t) override {
    	bool inArray = true;
    	saxContainer(inArray);
    	return true;
    }
    bool end_array() override {
	   	sax_context& top = sax_context::top();
	   	if (!top.nameList.empty()) {
	    	std::cout << "@@ " << sax_context::top().to_string() << std::endl;
	   	}

    	sax_context::pop();
    	return true;
    }


    //
    // binary ???
    //
    bool binary(json::binary_t& /*val*/) override {
    	DEBUG_TRACE();
    	ERROR();
        return true;
    }

    bool parse_error(std::size_t position, const std::string& last_token, const json::exception& ex) override {
    	logger.error("parser_error");
    	logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
    	ERROR();
        return false;
    }
};


int main(int, char**) {
	logger.info("START");
	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	// create a SAX event consumer object
	sax_handler handler;

	// parse JSON
	bool result = json::sax_parse(std::cin, &handler);
	logger.info("parse result = %d", result);

	logger.info("STOP");
	return 0;
}

/*
# macmini2020.lan
clang -Xclang -ast-dump=json -fsyntax-only -I /opt/local/include -I /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include -I /opt/local/libexec/qt6/include -I /opt/local/libexec/qt6/include/QtCore -std=c++17 src/main/main.cpp | LOG_CONFIG=tmp/cmake/macmini2020.lan/run/debug.properties tmp/cmake/macmini2020.lan/build/main/main

# dev-clang
clang -Xclang -ast-dump=json -fsyntax-only -I /usr/local/include -I /usr/local/include/qt6 -I /usr/local/include/qt6/QtCore -std=c++17 -fPIC src/main/main.cpp | LOG_CONFIG=tmp/cmake/dev-clang/run/debug.properties tmp/cmake/dev-clang/build/main/main
*/
