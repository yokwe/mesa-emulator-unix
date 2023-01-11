//
// JSON
//

#pragma once

#include <iostream>
#include <ios>
#include <limits>
#include <vector>
#include <utility>

namespace json {

class value {
	static std::string NULL_STRING;
	static std::string TRUE_STRING;
	static std::string FALSE_STRING;

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
public:

	std::string path;
	bool        inArray; // array or record
	int         arrayIndex;

	std::string getIndexName() {
		return std::to_string(arrayIndex++);
	}

	// default constructor
	container() :
		path(""), inArray(false), arrayIndex(0) {}
	container(std::string path_, bool inArray_, int arrayIndex_) :
		path(path_), inArray(inArray_), arrayIndex(arrayIndex_) {}
	container(bool inArray_) :
		path(""), inArray(inArray_), arrayIndex(0) {}

	// copy constructor
	container(const container& that) :
		path(that.path), inArray(that.inArray), arrayIndex(that.arrayIndex) {}
};


//
//
//
bool dump(std::istream& in);

}
