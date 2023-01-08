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

using json = nlohmann::json;

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("main");

class sax_value {
public:
	enum Type {
		NULL_, BOOL, INT, UINT, FLOAT, STRING,
	};

	static constexpr const char* NULL_STRING  = "NULL";
	static constexpr const char* TRUE_STRING  = "TRUE";
	static constexpr const char* FALSE_STRING = "FALSE";

	static const char* to_string(bool value) {
		return value ? TRUE_STRING : FALSE_STRING;
	}

private:
	const Type        type;
private:
	const bool        boolValue;
	const int64_t     intValue;
	const uint64_t    uintValue;
	const double      doubleValue;
	const std::string stringValue;

public:
	sax_value()                      : type(Type::NULL_), boolValue(false), intValue(0), uintValue(0), doubleValue(0), stringValue("") {}
	sax_value(bool newValue)         : type(Type::BOOL), boolValue(newValue), intValue(0), uintValue(0), doubleValue(0), stringValue("") {}
	sax_value(int64_t newValue)      : type(Type::INT), boolValue(false), intValue(newValue), uintValue((uint64_t)newValue), doubleValue(0), stringValue("") {}
	sax_value(uint64_t newValue)     : type(Type::UINT), boolValue(false), intValue((int64_t)newValue), uintValue(newValue), doubleValue(0), stringValue("") {}
	sax_value(double newValue, std::string newValueString) : type(Type::FLOAT), boolValue(false), intValue(0), uintValue(0), doubleValue(newValue), stringValue(newValueString) {}
	sax_value(std::string newValue)  : type(Type::STRING), boolValue(false), intValue(0), uintValue(0), doubleValue(0), stringValue(newValue) {}
	sax_value(const sax_value& that) : type(that.type), boolValue(that.boolValue), intValue(that.intValue), uintValue(that.uintValue), doubleValue(that.doubleValue), stringValue(that.stringValue) {}

	bool isNull() const {
		return type == Type::NULL_;
	}
	bool getBool() const {
		assert(type == Type::BOOL);
		return boolValue;
	}
	int64_t getInt() const {
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
	uint64_t getUInt() const {
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
	double getFloat() const {
		assert(type == Type::FLOAT);
		return doubleValue;
	}
	std::string getString() const {
		switch(type) {
		case Type::NULL_:
			return NULL_STRING;
		case Type::BOOL:
			return boolValue ? TRUE_STRING : "FALSE";
		case Type::INT:
			return std::to_string(intValue);
		case Type::UINT:
			return std::to_string(uintValue);
		case Type::FLOAT:
			return stringValue;
		case Type::STRING:
			return stringValue;
		default:
			logger.error("Unexpected type %d", type);
			ERROR();
		}
		return "";
	}
};


class sax_context {
public:
	static std::vector<sax_context> stack;

	static bool empty() {
		return stack.empty();
	}

	std::string path;
	bool        inArray; // array or record
	int         arrayIndex;

	std::string getIndexName() {
		return std::to_string(arrayIndex++);
	}

	sax_context() : path(""), inArray(false), arrayIndex(0) {}
	sax_context(std::string path_, bool inArray_, int arrayIndex_) : path(path_), inArray(inArray_), arrayIndex(arrayIndex_) {}
	sax_context(bool inArray_) : path(""), inArray(inArray_), arrayIndex(0) {}
	sax_context(const sax_context& that) : path(that.path), inArray(that.inArray), arrayIndex(that.arrayIndex) {}
};
std::vector<sax_context> sax_context::stack;


class sax_handler : public json::json_sax_t {
  public:
	std::string lastKey;

    //
    // value
    //
	std::string toLine(const std::string_view& value) {
	   	sax_context& back  = sax_context::stack.back();
		std::string  name  = back.inArray ? back.getIndexName() : lastKey;

		std::string ret = std_sprintf("%s/%s %s", back.path, name, value);
		return ret;
	}

    bool null() override {
    	auto value = sax_value::NULL_STRING;
    	auto line = toLine(value);
     	std::cout << line << std::endl;
        return true;
    }
    bool boolean(bool newValue) override {
    	auto value = sax_value::to_string(newValue);
    	auto line = toLine(value);
     	std::cout << line << std::endl;
        return true;
    }
    bool number_integer(number_integer_t newValue) override {
    	auto value = std::to_string(newValue);
       	auto line = toLine(value);
		std::cout << line << std::endl;
		return true;
    }
    bool number_unsigned(number_unsigned_t newValue) override {
    	auto value = std::to_string(newValue);
       	auto line = toLine(value);
		std::cout << line << std::endl;
		return true;
    }
    bool number_float(number_float_t , const string_t& newValueString) override {
    	auto value = newValueString;
       	auto line = toLine(value);
		std::cout << line << std::endl;
		return true;
    }
    bool string(string_t& newValue) override {
    	auto value = newValue;
       	auto line = toLine(value);
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
    // object
    //
    void container(bool inArray) {
    	if (sax_context::stack.empty()) {
    		sax_context context = sax_context("", inArray, 0);
    		sax_context::stack.push_back(context);
    	} else {
    	   	sax_context& back    = sax_context::stack.back();
    	   	std::string  name    = back.inArray ? back.getIndexName() : lastKey;
	   		sax_context  context = sax_context(back.path + "/" + name, inArray, 0);
	   		sax_context::stack.push_back(context);
    	}
    }
    bool start_object(std::size_t) override {
    	bool inArray = false;
    	container(inArray);
    	return true;
    }

    bool end_object() override {
    	sax_context::stack.pop_back();
    	return true;
    }

    //
    // array
    //
    bool start_array(std::size_t) override {
    	bool inArray = true;
    	container(inArray);
    	return true;
    }

    bool end_array() override {
    	sax_context::stack.pop_back();
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
