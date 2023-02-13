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
namespace json {
using token_t      = ::json::token_t;
using token_list_t = ::json::token_list_t;
using handler_t    = ::json::handler_t;

//


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
	void data(const token_t& token) override;
};


//
// split
//
class split_t : public pipe_t<token_t, token_list_t> {
	std::string  m_pattern;
	std::regex   m_regex;
	bool         m_has_value;
	token_list_t m_value;

public:
	split_t(source_t<token_t>* upstream, std::string glob) :
		pipe_t(__func__, upstream),
		m_pattern(::json::glob_to_regex(glob)),
		m_regex(std::regex(m_pattern)),
		m_has_value(false) {}
	~split_t() {
		base_t::close();
	}

	// source_t
	void         close_impl()    override;
	bool         has_next_impl() override;
	token_list_t next_impl()     override;
};
inline split_t split(source_t<token_t>* upstream, const char* glob) {
	return split_t(upstream, glob);
}


//
// expand
//
class expand_t : public pipe_t<token_list_t, token_t> {
	int          m_array_index;
	int          m_list_index;
	bool         m_has_value;
	bool         m_need_first_array;
	bool         m_need_last_leave;
	token_t      m_value;
	token_list_t m_list;
	std::string  m_array_name;
public:
	expand_t(source_t<token_list_t>* upstream) : pipe_t(__func__, upstream),
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
inline expand_t expand(source_t<token_list_t>* upstream) {
	return expand_t(upstream);
}


//
// include_path_value
//
filter_t<token_list_t> include_path_value(source_t<token_list_t>* upstream, const std::string& glob_path, const std::string& glob_value);


//
// exclide_path
//
filter_t<token_t> exclude_path(source_t<token_t>* upstream, std::initializer_list<std::string> args);

//
}
}
