//
// stream_json.h
//

#pragma once

#include "../util/Util.h"

#include "stream.h"
#include "json.h"

namespace stream {
//

using token_t   = json::token_t;
using handler_t = json::handler_t;

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
public:
	json_t(int max_queue_size = 100, int wait_time = 1) : source_t(__func__), m_max_queue_size(max_queue_size), m_wait_time(wait_time) {
		initialize();
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

	void parse(std::istream& in);

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


//
}
