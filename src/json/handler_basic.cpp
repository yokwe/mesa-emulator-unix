//
// handler_basic.cpp
//

#include "handler_basic.h"
#include "json_handler.h"


namespace json {
namespace handler {


void dump_basic(std::istream& in) {
	hander_basic_t handler;

	json_handler_t sax(&handler);

	sax.parse(in);
}


}
}
