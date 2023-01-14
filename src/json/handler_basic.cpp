//
// handler_basic.cpp
//

#include "handler_basic.h"
#include "json_handler.h"


namespace json {
namespace handler {


void dump_basic(std::istream& in) {
	handler_basic_t handler;

	json_handler_t json_handler(handler);

	json_handler.parse(in);
}


}
}
