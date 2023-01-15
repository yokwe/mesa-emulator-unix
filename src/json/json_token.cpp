//
//
//

#include "json_token.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-token");


namespace json {
namespace token {


bool json_token_t::binary(nlohmann::json::binary_t& /*val*/) {
	ERROR();
	return true;
}
bool json_token_t::parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) {
	logger.error("parser_error");
	logger.error("  position = %d  last token = %s  ex = %s", (int)position, last_token, ex.what());
	ERROR();
	return false;
}


class token_handler_dump_t : public token_handler_t {
public:
	void item (token_t& token) {
		std::cout << "ITEM " << token.name << " " << token.value << std::endl;
	}
	void enter(token_t& token) {
		std::cout << (token.arrayFlag ? "ARRAY  " : "OBJECT ") << token.name << std::endl;
	}
	void leave(token_t& token) {
		(void)token;
		std::cout << "LEAVE" << std::endl;
	}
};


void dump(std::istream& in) {
	token_handler_dump_t token_handler;

	json_token_t sax(token_handler);

	sax.parse(in);
}


}
}
