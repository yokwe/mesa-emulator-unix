//
// handler_dot.cpp
//

#include "handler_dot.h"
#include "json_handler.h"


namespace json {
namespace handler {


std::vector<handler_dot_t::context_t> handler_dot_t::m_stack;


void dump_dot(std::istream& in) {
	handler_dot_t handler;

	json_handler_t json_handler(handler);

	json_handler.parse(in);
}


}
}
