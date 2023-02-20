//
// stream_json.h
//

#pragma once

#include <string>
#include <deque>
#include <chrono>
#include <mutex>
#include <regex>


#include "stream.h"
#include "json.h"

namespace stream {
namespace json {
using token_t      = ::json::token_t;
using token_list_t = ::json::token_list_t;


//
// json
//
source_t<token_t> json(std::istream& in, int max_queue_size = 1000, int wait_time = 1);

//
// split
//
pipe_t<token_t, token_list_t> split(source_base_t<token_t>* upstream, const std::string& glob);

//
// expand
//
pipe_t<token_list_t, token_t> expand(source_base_t<token_list_t>* upstream);

//
//  include_path_value
//
pipe_t<token_list_t, token_list_t> include_path_value(
	source_base_t<token_list_t>* upstream, const std::string& glob_path, const std::string& glob_value);

//
// exclude_path()
//
struct exclude_path_predicate_t {
	std::regex m_regex;

	exclude_path_predicate_t(std::regex regex) : m_regex(regex) {}

	bool operator()(token_t token) const {
		// negate regex_match for exclude
		return !std::regex_match(token.path(), m_regex);
	}
};
template<typename ... Args>
pipe_t<token_t, token_t> exclude_path(source_base_t<token_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + ::json::glob_to_regex(e) + ")");
	}
	std::regex regex = std::regex(string.substr(1));

	auto predicate = exclude_path_predicate_t(regex);
	return stream::filter(upstream, predicate);
}

//
}
}
