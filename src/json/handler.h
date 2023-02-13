//
// handler.h
//

#pragma once

#include <iostream>

#include "json.h"


namespace json {


class null_handler_t : public handler_t {
public:
	void start() override {}
	void stop()  override {}

	void item (const token_t& token) override {
		(void)token;
	}
	void enter(const token_t& token) override {
		(void)token;
	}
	void leave(const token_t& token) override {
		(void)token;
	}
};


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
		std::cout << "ITEM   " << token.path() << " " << token.value() << std::endl;
	}
	void enter(const token_t& token) override {
		if (!only_item) std::cout << (token.is_array() ? "ARRAY  " : "OBJECT ") << token.path() << std::endl;
	}
	void leave(const token_t& token) override {
		if (!only_item) std::cout << "LEAVE  " << token.path() << std::endl;
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
		std::cout << pathList.back() << "/" << token.name() << " " << token.value() << std::endl;
	}
	void enter(const token_t& token) override {
		std::string path = pathList.empty() ? "" : (pathList.back() + "/" + token.name());
		pathList.push_back(path);
	}
	void leave(const token_t& token) override {
		(void)token;
		pathList.pop_back();
	}
};



}
