//
// token_record.h
//

#pragma once

#include <string>
#include <vector>

#include "json_token.h"


namespace json {
namespace token {


void dump_dot(const token_list_t& tokenList) {
	string_list_t pathList;

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
void dump_dot(const token_record_t& token_record) {
	std::cout << "##DUMP## " << token_record.path() << " " << token_record.item_count() << std::endl;

	dump_dot(token_record.token_list());
}


class json_token_record_t : public token_handler_t {
	token_record_t token_record;
	size_t         record_level;
	string_list_t  path_list;

public:
	void start() override {
		std::cout << "##START##" << std::endl;
		record_level = 3;
	}
	void stop() override {
		std::cout << "##STOP##" << std::endl;
	}

	void item (const token_t& token) override {
		token_record.add(token);
	}
	void enter(const token_t& token) override {
		// maintain pathList
		path_list.push_back(path_list.empty() ? "" : (path_list.back() + "/" + token.name));

		// maintain tokenList
		if (path_list.size() == record_level) {
			token_record.start(path_list.back());
		}

		token_record.add(token);
	}
	void leave(const token_t& token) override {
		token_record.add(token);

		// process tokenList
		if (path_list.size() == record_level) {
			if (!token_record.empty()) {
				dump_dot(token_record);
			}
		}

		// maintain pathList
		path_list.pop_back();
	}
};

void dump_record(std::istream& in) {
	json_token_record_t token_handler;

	json_token_t sax(token_handler);

	sax.parse(in);
}


}
}
