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
source_t<token_t>      json(std::istream& in, int max_queue_size = 1000, int wait_time = 1);

source_t<token_list_t> split(source_base_t<token_t>* upstream, const std::string& glob);

source_t<token_t>      expand(source_base_t<token_list_t>* upstream);

source_t<token_list_t> include_path_value(source_base_t<token_list_t>* upstream, const std::string& glob_path, const std::string& glob_value);

source_t<token_t>      exclude_path(source_base_t<token_t>* upstream, std::initializer_list<std::string> args);
source_t<token_t>      exclude_path(source_base_t<token_t>* upstream, std::string glob_path);

//
}
}
