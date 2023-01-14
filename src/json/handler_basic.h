//
//
//

#pragma once

#include <string>
#include <iostream>

#include "handler.h"


namespace json {
namespace handler {

	class handler_basic_t : public handler_t {
	public:
		void start() {
			std::cout << "START" << std::endl;
		}
		void stop() {
			std::cout << "STOP" << std::endl;
		}
		void item (const std::string& key, const std::string& value) {
			std::cout << "ITEM  " << key << " " << value << std::endl;
		}
		void enter(const std::string& key, const bool         isArray){
			std::cout << "ENTER " << (isArray ? "ARRAY " : "OBJECT") << " " << key << std::endl;
		}
		void leave() {
			std::cout << "LEAVE" << std::endl;
		}
	};

	void dump_basic(std::istream& in);

}
}
