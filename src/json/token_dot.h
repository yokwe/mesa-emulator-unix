//
// token_dot.h
//

#pragma once

#include <string>

#include "json_token.h"


namespace json {
namespace token {


class json_token_dot_t : public token_handler_t {
	string_list_t pathList;

public:
	void start() override {
		std::cout << "##START##" << std::endl;
	}
	void stop()  override {
		std::cout << "##STOP##" << std::endl;
	}

	void item (const token_t& token) override {
		std::cout << pathList.back() << "/" << token.name << " " << token.value << std::endl;
	}
	void enter(const token_t& token) override {
		std::string path = pathList.empty() ? "" : (pathList.back() + "/" + token.name);
		pathList.push_back(path);
	}
	void leave(const token_t& token) override {
		(void)token;
		pathList.pop_back();
	}
};

void dump_dot(std::istream& in) {
	json_token_dot_t token_handler;

	json_token_t sax(token_handler);

	sax.parse(in);
}


}
}
