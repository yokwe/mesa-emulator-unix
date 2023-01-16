//
// token_record.h
//

#pragma once

#include <string>
#include <vector>

#include "json_token.h"


namespace json {
namespace token {

typedef std::vector<token_t>     token_list_t;
typedef std::vector<std::string> string_list_t;


class record_token_t {
	std::string  m_path;
	int          m_item_count;
	token_list_t m_token_list;
public:
	void start(const std::string& path) {
		m_path       = path;
		m_item_count = 0;
		m_token_list.clear();
	}

	void add(const token_t& token) {
		m_token_list.push_back(token);
		if (token.type == token_t::Type::ITEM) m_item_count++;
	}

	bool empty() {
		return m_item_count == 0;
	}

	const std::string& path() const {
		return m_path;
	}
	int item_count() const {
		return m_item_count;
	}
	const std::vector<token_t>& token_list() const {
		return m_token_list;
	}

};


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
void dump_dot(const record_token_t& record_token) {
	std::cout << "##DUMP## " << record_token.path() << " " << record_token.item_count() << std::endl;

	dump_dot(record_token.token_list());
}


class json_token_record_t : public token_handler_t {
	record_token_t record_token;
	size_t         record_level;
	string_list_t  path_list;

public:
	void start() {
		std::cout << "##START##" << std::endl;
		record_level = 3;
	}
	void stop()  {
		std::cout << "##STOP##" << std::endl;
	}

	void item (token_t& token) {
		record_token.add(token);
	}
	void enter(token_t& token) {
		// maintain pathList
		path_list.push_back(path_list.empty() ? "" : (path_list.back() + "/" + token.name));

		// maintain tokenList
		if (path_list.size() == record_level) {
			record_token.start(path_list.back());
		}

		record_token.add(token);
	}
	void leave(token_t& token) {
		record_token.add(token);

		// process tokenList
		if (path_list.size() == record_level) {
			if (!record_token.empty()) {
				dump_dot(record_token);
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
