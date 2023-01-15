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

}
}
