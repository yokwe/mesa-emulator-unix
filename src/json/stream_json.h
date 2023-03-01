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

constexpr auto glob_to_regex = ::json::glob_to_regex;


//
// json - std::cin to token_t stream
//
source_t<token_t> json(std::istream& in, int max_queue_size = 1000, int wait_time = 1);


//
// split - convert token_t stream to token_list_t stream by glob path
//
pipe_t<token_t, token_list_t> split(source_base_t<token_t>* upstream, const std::string& glob);


//
// expand - convert token_list_t stream to token_t stream
//
pipe_t<token_list_t, token_t> expand(source_base_t<token_list_t>* upstream);


//
// file - save token_list_t stream as file
//
pipe_t<token_t, token_t> file(source_base_t<token_t>* upstream, const std::string& path);



//
// include_source - include only source files that match glob_path
// NOTE Source of data is output of "clang --asd-dump=json"
//
struct include_clang_source_function_t {
	static constexpr const char* token_path   = "/source";
	static constexpr const char* token_name   = token_path + 1;
	static constexpr const char* target_path  = "/loc/file";
	static constexpr const char* default_path = "default";

	std::regex   m_regex;
	std::string  m_source = default_path;
	token_list_t m_value;

	include_clang_source_function_t(const std::regex& regex) : m_regex(regex) {}
	~include_clang_source_function_t() {}

	token_list_t operator()(const token_list_t& list) {
	    assert(2 <= list.size());
	    token_t front = list.front();
	    token_t back  = list.back();

	    assert(front.is_start_object());
	    assert(back.is_end_object());

	    for(const auto& e: list) {
	    	// update m_source
	    	if (e.path() == target_path) {
    			m_source = e.value();
	    		logger.info("clang_include_source %s %s", (std::regex_match(m_source, m_regex) ? "*" : " "), m_source);
	    		break;
	    	}
	    }
	    // build m_value
		bool match = std::regex_match(m_source, m_regex);

	    token_t source = token_t::make_string(token_path, token_name, match ? m_source : default_path);
	    m_value.clear();
    	m_value.reserve(list.size() + 1);
	    if (match) {
	    	m_value = list;
	    	m_value.insert(m_value.begin() + 1, source);
	    } else {
		    m_value.push_back(front);
		    m_value.push_back(source);
		    m_value.push_back(back);
	    }
	    return m_value;
	}
};
template<typename ... Args>
pipe_t<token_list_t, token_list_t> include_clang_source(source_base_t<token_list_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}

	auto function = include_clang_source_function_t(std::regex(string.substr(1)));
	return stream::map(upstream, function);
}
pipe_t<token_list_t, token_list_t> exclude_clang_builtin_source(source_base_t<token_list_t>* upstream);


//
//  include_token_list_by_path_value
//
struct include_token_list_by_path_value_predicate_t {
	std::regex m_regex_path;
	std::regex m_regex_value;

	include_token_list_by_path_value_predicate_t(std::regex regex_path, std::regex regex_value) :
		m_regex_path (regex_path), m_regex_value(regex_value) {}

	bool operator()(const token_list_t& list) const {
	    for(const auto& e: list) {
	    	if (std::regex_match(e.path(), m_regex_path) && std::regex_match(e.value(), m_regex_value)) return true;
	    }
	    return false;
	}
};
template<typename ... Args>
pipe_t<token_list_t, token_list_t> include_token_list_by_path_value(source_base_t<token_list_t>* upstream, const std::string& glob_path, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}
	auto predicate = include_token_list_by_path_value_predicate_t(std::regex(glob_to_regex(glob_path)), std::regex(string.substr(1)));
	return stream::filter(upstream, predicate);
}


//
//  exclude_token_list_by_path_value
//
struct exclude_token_list_by_path_value_predicate_t {
	std::regex m_regex_path;
	std::regex m_regex_value;

	exclude_token_list_by_path_value_predicate_t(const std::regex& regex_path, const std::regex& regex_value) :
		m_regex_path (regex_path), m_regex_value(regex_value) {}

	bool operator()(const token_list_t& list) {
	    for(const auto& token: list) {
	    	if (std::regex_match(token.path(), m_regex_path) && std::regex_match(token.value(), m_regex_value)) return false;
	    }
	    return true;
	}
};
template<typename ... Args>
pipe_t<token_list_t, token_list_t> exclude_token_list_by_path_value(source_base_t<token_list_t>* upstream, const std::string& glob_path, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}
	auto predicate = exclude_token_list_by_path_value_predicate_t(std::regex(glob_to_regex(glob_path)), std::regex(string.substr(1)));
	return stream::filter(upstream, predicate);
}


//
// exclude_token_list_by_path - exclude token_t that match args (glob)
//
struct exclude_token_by_path_function_t {
	std::regex   m_regex;
	token_list_t m_result;

	exclude_token_by_path_function_t(std::regex regex) : m_regex(regex) {}

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
pipe_t<token_list_t, token_list_t> exclude_token_by_path(source_base_t<token_list_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}

	auto function = exclude_token_by_path_function_t(std::regex(string.substr(1)));
	return stream::map(upstream, function);
}


//
// include_token_list_by_path - include token_t that match args (glob)
//
struct include_token_by_path_function_t {
	std::regex   m_regex;
	token_list_t m_result;

	include_token_by_path_function_t(std::regex regex) : m_regex(regex) {}

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
pipe_t<token_list_t, token_list_t> include_token_by_path(source_base_t<token_list_t>* upstream, Args&& ... args) {
	std::string string;
	for (auto e : std::initializer_list<std::string>{args...}) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}

	auto function = include_token_by_path_function_t(std::regex(string.substr(1)));
	return stream::map(upstream, function);
}


//
}
}
