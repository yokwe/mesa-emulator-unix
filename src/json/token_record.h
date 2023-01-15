//
// token_record.h
//

#pragma once

#include <string>
#include <vector>

#include "json_token.h"


namespace json {
namespace token {


class token_list_dump {
	std::vector<std::string> pathList;

public:
	void dump(const std::vector<token_t>& tokenList) {
		pathList.clear();

		std::cout << "----" << std::endl;
		for(auto token: tokenList) {
			switch(token.type) {
			case token_t::Type::ITEM:
				std::cout << pathList.back() << "/" << token.name << " " << token.value << std::endl;
				break;
			case token_t::Type::ENTER:
				pathList.push_back(pathList.empty() ? "" : (pathList.back() + "/" + token.name));
				break;
			case token_t::Type::LEAVE:
				pathList.pop_back();
				break;
			default:
				break;
			}
		}
		std::cout << "----" << std::endl;
	}
};

class json_token_record_t : public token_handler_t {
	int                  level;
	int                  itemCount;
	std::vector<token_t> tokenList;

	token_list_dump      list_dump;

public:
	void start() {
		std::cout << "##START##" << std::endl;
		level = 0;
		itemCount = 0;
	}
	void stop()  {
		std::cout << "##STOP##" << std::endl;
	}

	void item (token_t& token) {
		tokenList.push_back(token);
		itemCount++;
	}
	void enter(token_t& token) {
		level++;

		if (level == 3) {
			tokenList.clear();
			itemCount = 0;
		}
		tokenList.push_back(token);
	}
	void leave(token_t& token) {
		tokenList.push_back(token);

		if (level == 3) {
			if (0 < itemCount) {
				list_dump.dump(tokenList);
			}
		}

		level--;
	}
};

void dump_record(std::istream& in) {
	json_token_record_t token_handler;

	json_token_t sax(token_handler);

	sax.parse(in);
}


}
}
