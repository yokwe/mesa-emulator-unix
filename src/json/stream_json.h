//
// stream_json.h
//

#pragma once

#include <string>
#include <regex>
#include <initializer_list>
#include <tuple>

#include "../util/Util.h"

#include "stream.h"
#include "json.h"

namespace stream {
//

using token_t   = json::token_t;
using handler_t = json::handler_t;
using token_list_t = json::token_list_t;

class json_t : public handler_t, public source_t<token_t> {
	using seconds = std::chrono::seconds;

	const int     m_max_queue_size;
	const seconds m_wait_time;

	bool    m_end_of_stream;
	bool    m_stop_capture;
	bool    m_thread_active;
	int     m_count_token;
	int     m_count_capture;
	int     m_count_next;

	std::deque<token_t>     m_queue;

	std::mutex              m_mutex;
	std::condition_variable m_fill_queue;
	std::condition_variable m_queue_is_filled;

	void initialize() {
		m_end_of_stream  = false;
		m_stop_capture   = false;
		m_thread_active  = false;
		m_count_token    = 0;
		m_count_capture  = 0;
		m_count_next     = 0;

		m_queue.clear();
	}
	void parse(std::istream& in);
public:
	json_t(std::istream& in, int max_queue_size = 1000, int wait_time = 1) : source_t(__func__), m_max_queue_size(max_queue_size), m_wait_time(wait_time) {
		parse(in);
	}

	~json_t() {
		base_t::close();
	}

	bool thread_active() {
		return m_thread_active;
	}
	void thread_active(bool newValue) {
		m_thread_active = newValue;
	}

	//
	// token_t
	//
	void    close_impl()    override;
	bool    has_next_impl() override;
	token_t next_impl()     override;

	//
	// source_t<token_t>
	//
	// life cycle event
	void start() override;
	void stop()  override;

	// data event
	void process_token(const token_t& token);
	void item (const token_t& token) override {
		process_token(token);
	}
	void enter(const token_t& token) override {
		process_token(token);
	}
	void leave(const token_t& token) override {
		process_token(token);
	}
};


class json_split_t : public source_t<token_list_t> {
	source_t<token_t>* m_upstream;
	std::string        m_pattern;
	std::regex         m_regex;
	bool               m_has_value;
	token_list_t       m_value;

public:
	json_split_t(source_t<token_t>* upstream, std::string glob) :
		source_t(__func__),
		m_upstream(upstream),
		m_pattern(json::glob_to_regex(glob)),
		m_regex(std::regex(m_pattern)),
		m_has_value(false) {}
	~json_split_t() {
		base_t::close();
	}

	// source_t
	void         close_impl()    override;
	bool         has_next_impl() override;
	token_list_t next_impl()     override;
};


class json_expand_t : public source_t<token_t> {
	source_t<token_list_t>* m_upstream;
	int                     m_array_index;
	int                     m_list_index;
	bool                    m_has_value;
	bool                    m_need_first_array;
	bool                    m_need_last_leave;
	token_t                 m_value;
	token_list_t            m_list;
	std::string             m_array_name;
public:
	json_expand_t(source_t<token_list_t>* upstream) : source_t(__func__),
		m_upstream(upstream),
		m_array_index(0),
		m_list_index(0),
		m_has_value(false),
		m_need_first_array(true),
		m_need_last_leave(true) {}

	// source_t
	void    close_impl()    override;
	bool    has_next_impl() override;
	token_t next_impl()     override;
};


template<typename T=token_t>
json_split_t split(source_t<T>* upstream, const char* glob) {
	return json_split_t(upstream, glob);
}
template<typename T=token_list_t>
json_expand_t expand(source_t<T>* upstream) {
	return json_expand_t(upstream);
}


template<typename T=token_list_t>
class json_include_path_value_predicate_t {
	std::regex regex_path;
	std::regex regex_value;
public:
	json_include_path_value_predicate_t(const std::string& glob_path, const std::string& glob_value) :
		regex_path(json::glob_to_regex(glob_path)),
		regex_value(json::glob_to_regex(glob_value)) {}

	bool operator()(T list) const {
	    for(const auto& e: list) {
	    	if (std::regex_match(e.path, regex_path) && std::regex_match(e.value, regex_value)) return true;
	    }
	    return false;
	}
};
template<typename T=token_list_t>
filter_t<T> include_path_value(source_t<T>* upstream, const std::string& glob_path, const std::string& glob_value) {
	json_include_path_value_predicate_t<T> predicate(glob_path, glob_value);
	return filter(upstream, predicate);
}


template<typename T=token_t, class... Args>
class json_exclude_path_predicate_t {
	std::regex m_regex;
public:
	json_exclude_path_predicate_t(Args... args) {
		std::tuple<Args...> tuple(args...);

		std::vector<std::string> regex_string_list;
		{
			auto func = [&](auto&&... args){(regex_string_list.push_back(json::glob_to_regex(args)), ...);};
			std::apply(func, tuple);
		}

		std::string regex_string;
		for(int i = 0; i < (int)regex_string_list.size(); i++) {
			if (1 <= i) regex_string.append("|");
			regex_string.append("(?:" + regex_string_list.at(i) + ")");
		}

		m_regex = std::regex(regex_string);
	}
	json_exclude_path_predicate_t(std::regex regex) : m_regex(regex) {}
	json_exclude_path_predicate_t(const json_exclude_path_predicate_t& that) : m_regex(that.m_regex) {}

	bool operator()(T token) const {
		// negate regex_match for exclude
		return !std::regex_match(token.path, m_regex);
	}
};
template<typename T=token_t, class... Args>
filter_t<T> exclude_path(source_t<T>* upstream, Args... args) {
	json_exclude_path_predicate_t predicate(args...);
	return filter(upstream, predicate);
}

//
}
