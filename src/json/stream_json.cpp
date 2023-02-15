//
//
//

#include <thread>
#include <chrono>
#include <mutex>
#include <deque>

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("stream_json");

#include "stream_json.h"


//constexpr auto glob_to_regex = json::glob_to_regex;


namespace stream {
namespace json {


//
// json
//
struct json_impl_t : public ::json::handler_t, public source_base_t<token_t> {
	using seconds = std::chrono::seconds;

	const int     m_max_queue_size;
	const seconds m_wait_time;

	bool    m_end_of_stream = false;
	bool    m_stop_capture  = false;
	bool    m_thread_active = false;
	int     m_count_token   = 0;
	int     m_count_capture = 0;
	int     m_count_next    = 0;

	std::deque<token_t>     m_queue;

	std::mutex              m_mutex;
	std::condition_variable m_fill_queue;
	std::condition_variable m_queue_is_filled;

	json_impl_t(std::istream& in, int max_queue_size, int wait_time) : m_max_queue_size(max_queue_size), m_wait_time(wait_time) {
		parse(in);
	}

	static void parse_impl(std::istream* in, json_impl_t* json) {
		logger.info("parse thread start");
		json->m_thread_active = true;
		::json::parse(*in, json);
		json->m_thread_active = false;
		logger.info("parse thread stop");
	}

	void parse(std::istream& in) {
		m_end_of_stream  = false;
		m_stop_capture   = false;
		m_thread_active  = false;
		m_count_token    = 0;
		m_count_capture  = 0;
		m_count_next     = 0;

		// start thread
		auto thread = std::thread(parse_impl, &in, this);
		// detach thread
		thread.detach();
	}

	//
	// source_base_t<token_t>
	//
	void close() override {
		if (m_thread_active) {
			logger.info("close thread is active");
			m_stop_capture = true;
			m_fill_queue.notify_one();
			for(int i = 0; i < 3; i++) {
				logger.info("close sleep %d", i);
				std::this_thread::sleep_for(m_wait_time);
				if (m_thread_active) break;
			}
		}

		logger.info("close end of stream  %s",  m_end_of_stream ? "T" : "F");
		logger.info("close stop   capture %s",  m_stop_capture ? "T" : "F");
		logger.info("close thread active  %s",  m_thread_active ? "T" : "F");
		logger.info("close count token    %10d", m_count_token);
		logger.info("close count capture  %10d", m_count_capture);
		logger.info("close count next     %10d", m_count_next);
	}
	bool has_next() override {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_queue.empty()) {
			for(;;) {
				if (m_end_of_stream) break;
				if (m_stop_capture)  break;
				// notify item/enter/leave
				m_fill_queue.notify_one();
				// wait notification or timeout
				m_queue_is_filled.wait_for(lock, m_wait_time);
				// if there is data in m_queue, return true;
				if (!m_queue.empty()) return true;
			}
			return false;
		} else {
			return true;
		}
	}
	token_t next() override {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_queue.empty()) {
			logger.error("call next while m_queue.empty() is error");
			ERROR();
		} else {
			m_count_next++;
			auto ret = m_queue.back();
			m_queue.pop_back();
			return ret;
		}
	}

	//
	// token_h
	//
	void start() override {
		logger.info("json start");
		m_end_of_stream = false;
	}
	void stop() override {
		logger.info("json stop");
		m_end_of_stream = true;
		m_stop_capture  = true;

		m_queue_is_filled.notify_one();
	}
	void data(const token_t& token) override {
		m_count_token++;
		if (m_stop_capture) return;

		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_max_queue_size <= (int)m_queue.size()) {
			// notify queue become full
			m_queue_is_filled.notify_one();
			// wait until m_queue become empty or m_stop_capture
			for(;;) {
				m_fill_queue.wait_for(lock, m_wait_time);
				if (m_stop_capture) return;
				if (m_queue.empty()) break;
			}
		}
		m_count_capture++;
		m_queue.push_front(token);
	}
};
source_t<token_t> json(std::istream& in, int max_queue_size, int wait_time) {
	auto impl = new json_impl_t(in, max_queue_size, wait_time);
	return source_t<token_t>(impl, __func__);
}

// FIXME add split and expand
// FIXME add include_path_value exlclude_path


}
}
