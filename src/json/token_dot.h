//
// token_dot.h
//

#pragma once

#include <string>
#include <vector>

#include "json_token.h"


namespace json {
namespace token {


class json_token_dot_t : public token_handler_t {
public:
	std::vector<std::string> pathList;


	void start() {
		std::cout << "##START##" << std::endl;
	}
	void stop()  {
		std::cout << "##STOP##" << std::endl;
	}

	void item (token_t& token) {
		std::cout << pathList.back() << "/" << token.name << " " << token.value << std::endl;
	}
	void enter(token_t& token) {
		std::string path = pathList.empty() ? "" : (pathList.back() + "/" + token.name);
		pathList.push_back(path);
	}
	void leave(token_t& token) {
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
