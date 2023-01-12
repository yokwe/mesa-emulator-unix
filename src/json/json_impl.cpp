//
//
//

#include "json_impl.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-impl");

namespace json {
namespace impl {


//
// utility function
//
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


//
// json_value
//
const std::string json_value::NULL_STRING  = "NULL";
const std::string json_value::TRUE_STRING  = "TRUE";
const std::string json_value::FALSE_STRING = "FALSE";


//
// json_sax
//
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

}
}

