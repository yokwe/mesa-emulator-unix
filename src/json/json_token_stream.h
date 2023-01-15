//
// json_token_stream.h
//

#pragma once

#include <string>
#include <iostream>

#include "json_token.h"


namespace json {
namespace token {


class json_token_stream {
public:
	std::istream& m_in;
	int m_buffer_size;

	json_token_stream(std::istream& in_, int buffer_size_) : m_in(in_), m_buffer_size(buffer_size_) {
		start_thread();
	}
	~json_token_stream() {
		stop_thread();
	}

	// start thread
	void start_thread();

	// stop thread
	void stop_thread();

	// java style iterator
	bool    has_next();
	token_t next();

};


}
}
