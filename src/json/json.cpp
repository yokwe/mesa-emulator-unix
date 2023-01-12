//
// json.cpp
//

#include <regex>

#include "json.h"

#include "../util/Util.h"
const Logger logger = Logger::getLogger("json");

namespace json {

//
// static variable definition
//
const std::string value::NULL_STRING  = "NULL";
const std::string value::TRUE_STRING  = "TRUE";
const std::string value::FALSE_STRING = "FALSE";


bool json_sax::binary(nlohmann::json::binary_t& /*val*/) {
	ERROR();
	return true;
}
bool json_sax::parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) {
	logger.error("parser_error");
	logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
	ERROR();
	return false;
}


std::string glob_to_regex(std::string glob) {
	std::string ret;
	for(size_t i = 0; i < glob.size(); i++) {
		char c = glob[i];
		switch(c) {
		case '?':
			ret += "[^/]";
			break;
		case '*':
			if ((i + 1) == glob.size()) {
				ret += "[^/]*?"; // *
			} else {
				char next = glob[i + 1];
				if (next == '*') {
					ret += ".*?"; // **
					i++;
				} else {
					ret += "[^/]*?"; // *
				}
			}
			break;
		case '+':
		case '^':
		case '$':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '|':
		case '\\':
			ret += '\\';
			ret += c;
			break;
		default:
			ret += c;
			break;
		}
	}
	return ret;
}


class json_dump : public json_sax {
public:
	std::vector<std::regex> pathFilters;

	void addPathFilter(std::string glob) {
		std::string regex = glob_to_regex(glob);
		std::regex re(regex);
		pathFilters.push_back(re);
	}
	bool matchPathFilter(std::string& path) {
		for(auto re: pathFilters) {
			if (std::regex_match(path, re)) return true;
		}
		return false;
	}

	//
	// value
	//
	void process(const value& value) {
		if (top().filter()) return;

		std::string path = top().getPath(lastKey);
		if (matchPathFilter(path)) {
			// filtered
		} else {
			std::string line = path + " " + value.to_string();
			std::cout << line << std::endl;
		}
	}

	//
	// container
	//
	void process(bool inArray) {
		if (empty()) {
			container container("", inArray);
			push(container);
		} else {
			std::string path = top().getPath(lastKey);
			container container(path, inArray);
			if (matchPathFilter(path)) container.setFilter();
			push(container);
		}
	}
};


bool dump(std::istream& in) {
	json_dump sax;

	sax.addPathFilter("**Id");
	sax.addPathFilter("**/range/**");
	sax.addPathFilter("**/is*");
	sax.addPathFilter("**/loc/includedFrom/**");
	sax.addPathFilter("**/loc/offset");
	sax.addPathFilter("**/loc/line");
	sax.addPathFilter("**/loc/col");
	sax.addPathFilter("**/loc/tokLen");
	sax.addPathFilter("**/loc/spellingLoc/**");
	sax.addPathFilter("**/loc/expansionLoc/**");
	sax.addPathFilter("**/definitionData/**");
	sax.addPathFilter("**/bases/**");

	return nlohmann::json::sax_parse(in, &sax);
}

}
