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
// include_file
//
pipe_t<token_list_t, token_list_t> include_file(source_base_t<token_list_t>* upstream, const std::string& glob_path);

//
// exclude_path()
//
// For token_t
struct exclude_path_predicate_t {
	std::regex m_regex;

	exclude_path_predicate_t(std::regex regex) : m_regex(regex) {}

	bool operator()(const token_t& token) const {
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
// For token_list_t
struct exclude_path_function_t {
	std::regex   m_regex;
	token_list_t m_result;

	exclude_path_function_t(std::regex regex) : m_regex(regex) {}

	token_list_t operator()(const token_list_t& list) {
		m_result.clear();
		for(const token_t& token: list) {
			if (std::regex_match(token.path(), m_regex)) {
				// token path match condition => exclude
			} else {
				m_result.push_back(token);
			}
		}
		return m_result;
	}
};
template<typename ... Args>
pipe_t<token_list_t, token_list_t> exclude_path(source_base_t<token_list_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + ::json::glob_to_regex(e) + ")");
	}
	std::regex regex = std::regex(string.substr(1));

	auto function = exclude_path_function_t(regex);
	return stream::map(upstream, function);
}


//
// include_path
//
// For token_t
struct include_path_predicate_t {
	std::regex m_regex;

	include_path_predicate_t(std::regex regex) : m_regex(regex) {}

	bool operator()(const token_t& token) const {
		return token.is_item() ? std::regex_match(token.path(), m_regex) : true;
	}
};
template<typename ... Args>
pipe_t<token_t, token_t> include_path(source_base_t<token_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + ::json::glob_to_regex(e) + ")");
	}
	std::regex regex = std::regex(string.substr(1));

	auto predicate = include_path_predicate_t(regex);
	return stream::filter(upstream, predicate);
}
// For token_list_t
struct include_path_function_t {
	std::regex   m_regex;
	token_list_t m_result;

	include_path_function_t(std::regex regex) : m_regex(regex) {}

	token_list_t operator()(const token_list_t& list) {
		m_result.clear();
		for(const token_t& token: list) {
			if (token.is_item()) {
				if (std::regex_match(token.path(), m_regex)) {
					m_result.push_back(token);
				} else {
					//
				}
			} else {
				m_result.push_back(token);
			}
		}

		return m_result;
	}
};
template<typename ... Args>
pipe_t<token_list_t, token_list_t> include_path(source_base_t<token_list_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + ::json::glob_to_regex(e) + ")");
	}
	std::regex regex = std::regex(string.substr(1));

	auto function = include_path_function_t(regex);
	return stream::map(upstream, function);
}


//
// file - save stream to file
//
pipe_t<token_t, token_t> file(source_base_t<token_t>* upstream, const std::string& path);

//
}
}
