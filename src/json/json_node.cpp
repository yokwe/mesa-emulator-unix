//
//
//

#include "json_node.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-node");

namespace json {
namespace node {


item_t::~item_t() {
	if (valueNodeFree) delete valueNode;
}


std::string& item_t::getString() {
	if (isNode()) {
		logger.error("item is node");
		ERROR();
	}
	return valueString;
}


node_t& item_t::getNode() {
	if (!isNode()) {
		logger.error("item is string");
		ERROR();
	}
	return *valueNode;
}


}
}
