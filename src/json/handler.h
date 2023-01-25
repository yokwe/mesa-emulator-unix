//
// handler.h
//

#pragma once

#include <iostream>

#include "json.h"


namespace json {


class basic_handler_t : public handler_t {
	bool only_item;
public:
	basic_handler_t(bool only_item_ = false) : only_item(only_item_) {}

	void start() override {
		std::cout << "START" << std::endl;
	}
	void stop()  override {
		std::cout << "STOP" << std::endl;
	}

	void item (const token_t& token) override {
		std::cout << "ITEM   " << token.path << " " << token.value << std::endl;
	}
	void enter(const token_t& token) override {
		if (!only_item) std::cout << (token.arrayFlag ? "ARRAY  " : "OBJECT ") << token.path << std::endl;
	}
	void leave(const token_t& token) override {
		if (!only_item) std::cout << "LEAVE  " << token.path << std::endl;
	}
};


class dot_handler_t : public handler_t {
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


class block_handler_t : public handler_t {
	int          record_level;
	handler_t*   handler;
	//
	int          count_item;
	int          count_level;
	token_list_t token_list;

public:
	block_handler_t(int record_level_, handler_t* handler_) :
		record_level(record_level_), handler(handler_), count_item(0), count_level(0) {}

	void start() override {
//		std::cout << "## block_handler_t START ##" << std::endl;
		count_item   = 0;
	}
	void stop() override {
//		std::cout << "## block_handler_t STOP ##" << std::endl;
	}

	void item (const token_t& token) override {
		// capture token
		token_list.push_back(token);
		// maintain count_item
		count_item++;
	}
	void enter(const token_t& token) override {
		// maintain count_level
		count_level++;
		// maintain tokenList
		if (count_level == record_level) {
			token_list.clear();
			count_item = 0;
		}

		// capture token
		token_list.push_back(token);
	}
	void leave(const token_t& token) override {
		token_list.push_back(token);

		// process tokenList
		if (count_level == record_level) {
			if (0 < count_item) {
				std::cout << "## block_handler_t HANDLER START ##" << std::endl;
				parse(token_list, handler);
				std::cout << "## block_handler_t HANDLER STOP ##" << std::endl;
			}
		}

		// maintain count_level
		count_level--;
	}
};



}
