//
//
//

#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <deque>

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("stream_json");

#include "stream_json.h"


using token_t   = json::token_t;
using handler_t = json::handler_t;
using seconds   = std::chrono::seconds;

namespace stream {


void parse_impl(std::istream* in, json_t* json) {
	logger.info("parse thread start");
	json->thread_active(true);
	json::parse(*in, json);
	json->thread_active(false);
	logger.info("parse thread stop");
}
void json_t::parse(std::istream& in) {
	// initialize variables
	initialize();
	// start thread
	auto thread = std::thread(parse_impl, &in, this);
	// detach thread
	thread.detach();
}


void json_t::close_impl() {
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
bool json_t::has_next_impl() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_queue.empty()) {
		for(;;) {
			if (m_end_of_stream) return false;
			if (m_stop_capture)  return false;
			// notify item/enter/leave
			m_fill_queue.notify_one();
			// wait notification or timeout
			m_queue_is_filled.wait_for(lock, m_wait_time);
			// if there is data in m_queue, return true;
			if (!m_queue.empty()) return true;
		}
	} else {
		return true;
	}
}
token_t json_t::next_impl() {
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

void json_t::start() {
	logger.info("json token start");
	m_end_of_stream = false;
}
void json_t::stop() {
	logger.info("json_token stop");
	m_end_of_stream = true;
	m_stop_capture  = true;

	m_queue_is_filled.notify_one();
}

void json_t::process_token(const token_t& token) {
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


#if 0
json_t parse(std::istream& in) {
	json_t json;
	json.parse(in);
	return json;
}
#endif


}
