//
// token_basic.h
//

#pragma once

#include <string>
#include <iostream>

#include "json_token.h"

namespace json {
namespace token {


class json_token_basic_t : public token_handler_t {
public:
	void start() override {
		std::cout << "START" << std::endl;
	}
	void stop()  override {
		std::cout << "STOP" << std::endl;
	}

	void item (const token_t& token) override {
		std::cout << "ITEM " << token.name << " " << token.value << std::endl;
	}
	void enter(const token_t& token) override {
		std::cout << (token.arrayFlag ? "ARRAY  " : "OBJECT ") << token.name << std::endl;
	}
	void leave(const token_t& token) override {
		(void)token;
		std::cout << "LEAVE" << std::endl;
	}
};

void dump_basic(std::istream& in) {
	json_token_basic_t token_handler;

	json_token_t sax(token_handler);

	sax.parse(in);
}



}
}
