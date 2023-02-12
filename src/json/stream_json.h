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

using token_t      = json::token_t;
using handler_t    = json::handler_t;
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


class json_split_t : public pipe_t<token_t, token_list_t> {
	std::string  m_pattern;
	std::regex   m_regex;
	bool         m_has_value;
	token_list_t m_value;

public:
	json_split_t(source_t<token_t>* upstream, std::string glob) :
		pipe_t(__func__, upstream),
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


class json_expand_t : public pipe_t<token_list_t, token_t> {
	int          m_array_index;
	int          m_list_index;
	bool         m_has_value;
	bool         m_need_first_array;
	bool         m_need_last_leave;
	token_t      m_value;
	token_list_t m_list;
	std::string  m_array_name;
public:
	json_expand_t(source_t<token_list_t>* upstream) : pipe_t(__func__, upstream),
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


template<typename T>
json_split_t split(source_t<T>* upstream, const char* glob) {
	static_assert(std::is_same_v<T, token_t>);

	return json_split_t(upstream, glob);
}
template<typename T>
json_expand_t expand(source_t<T>* upstream) {
	static_assert(std::is_same_v<T, token_list_t>);

	return json_expand_t(upstream);
}


template<typename T>
class json_include_path_value_predicate_t {
	static_assert(std::is_same_v<T, token_list_t>);

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
template<typename T>
filter_t<T> include_path_value(source_t<T>* upstream, const std::string& glob_path, const std::string& glob_value) {
	static_assert(std::is_same_v<T, token_list_t>);

	json_include_path_value_predicate_t<T> predicate(glob_path, glob_value);
	return filter(upstream, predicate);
}


template<typename T>
class json_exclude_path_predicate_t {
	static_assert(std::is_same_v<T, token_t>);

	std::regex m_regex;
public:
	json_exclude_path_predicate_t(std::initializer_list<std::string> args) {
		assert(args.size() != 0);

		std::string string;
		for(auto e: args) {
			string.append("|(?:" + json::glob_to_regex(e) + ")");
		}
		m_regex = std::regex(string.substr(1));
	}

	bool operator()(T token) const {
		// negate regex_match for exclude
		return !std::regex_match(token.path, m_regex);
	}
};
template<typename T>
filter_t<T> exclude_path(source_t<T>* upstream, std::initializer_list<std::string> args) {
	static_assert(std::is_same_v<T, token_t>);

	json_exclude_path_predicate_t<T> predicate(args);
	return filter(upstream, predicate);
}

//
}
