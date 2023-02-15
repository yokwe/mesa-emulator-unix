//
// stream_json.h
//

#pragma once

#include <string>
#include <deque>
#include <chrono>
#include <mutex>


#include "stream.h"
#include "json.h"

namespace stream {
namespace json {
using token_t      = ::json::token_t;
using token_list_t = ::json::token_list_t;


// function
source_t<token_t> json(std::istream& in, int max_queue_size = 1000, int wait_time = 1);

// FIXME add split and expand
// FIXME add include_path_value exlclude_path

//
}
}
