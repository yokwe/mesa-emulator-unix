//
// handler.h
//

#pragma once

#include <string>
#include <iostream>

namespace json {


class handler_t {
public:
	virtual ~handler_t() {}

	virtual void start() = 0;
	virtual void stop()  = 0;
	virtual void item (const std::string& key, const std::string& value)   = 0;
	virtual void enter(const std::string& key, const bool         isArray) = 0;
	virtual void leave() = 0;
};


}
