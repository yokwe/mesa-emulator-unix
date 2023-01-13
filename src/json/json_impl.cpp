//
//
//

#include "json_impl.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-impl");

namespace json {
namespace impl {


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

