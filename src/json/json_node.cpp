//
//
//

#include "json_node.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-node");

namespace json {
namespace node {


std::vector<json_node::context> json_node::context::stack;

bool json_node::binary(nlohmann::json::binary_t& /*val*/) {
	ERROR();
	return true;
}
bool json_node::parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) {
	logger.error("parser_error");
	logger.error("  position = %d  last token = %s  ex = %s", (int)position, last_token, ex.what());
	ERROR();
	return false;
}


int dump(std::istream& in) {
	json_node sax;

	return sax.parse(in);
}

}
}
