//
//
//

#include "json_node.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-node");

namespace json {
namespace node {


std::string& item_t::getString() {
	if (isNode()) {
		logger.error("item is node");
		ERROR();
	}
	return valueString;
}


std::shared_ptr<node_t> item_t::getNode() {
	if (!isNode()) {
		logger.error("item is string");
		ERROR();
	}
	return valueNode;
}


}
}
