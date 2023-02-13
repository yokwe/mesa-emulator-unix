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

// stream_json.h includes stream.h
// stream.h use variable logger
// stream.h needs after ../util/Util.h
#include "stream_json.h"


using token_t      = json::token_t;
using token_list_t = json::token_list_t;
using handler_t    = json::handler_t;
using seconds      = std::chrono::seconds;


namespace stream {


//
// json_t
//
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

// source_t<token_t>
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
// handler_t
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
void json_t::data(const token_t& token) {
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


//
// json_split_t
//
// soruce_t<token_list_t>
void json_split_t::close_impl() {}
bool json_split_t::has_next_impl() {
	if (m_has_value) return true;

	// build m_has_value and m_value
	std::string path("//");
	int prefix = 0;
	bool capturing = false;
	for(;;) {
		// reach end of stream
		if (!m_upstream->has_next()) break;

		token_t token = m_upstream->next();
		if (capturing) {
			m_value.emplace_back(token, token.path().substr(prefix));
		}

		if (token.is_item()) {
			// OK
		} else if (token.is_start()) {
			if (std::regex_match(token.path(), m_regex)) {
				// found interest node
				path = token.path();
				prefix = path.size();
				capturing = true;
				//
				m_value.clear();
				m_value.emplace_back(token, token.path().substr(prefix));
			}
		} else if (token.is_end()) {
			if (token.path() == path) {
				// end of interest node
				path = "//";
				prefix = 0;
				capturing = false;
				//
				m_has_value = true;
				return true;
			}
		} else {
			ERROR();
		}
	}
	return false;
}
token_list_t json_split_t::next_impl() {
	if (m_has_value) {
		m_has_value = false;
		return m_value;
	} else {
		// if there is no next and call next(), it is error
		logger.error("there is no next and call next() is error");
		ERROR();
		return m_value;
	}
}


//
// json_exand_t
//
// source_t<token_t>

// FIXME need to return root array
void json_expand_t::close_impl() {
	// FIXME
}
bool json_expand_t::has_next_impl() {
	if (m_has_value) return true;

	if (m_need_first_array) {
		m_need_first_array = false;
		//
		// start of root array
		//
		m_value = token_t::make_start_array("", "");
		m_has_value = true;
		return true;
	}

	// m_list is empty, fill with upstream
	if (m_list.empty()) {
		if (m_upstream->has_next()) {
			m_list = m_upstream->next();
			m_array_name = std::to_string(m_array_index);
			m_list_index = 0;
		} else {
			// no next data
			if (m_need_last_leave) {
				m_need_last_leave = false;
				//
				// end of root array
				//
				m_value = token_t::make_end_array("", "");
				m_has_value = true;
				return true;
			}
			return false;
		}
	}

	if (0 <= m_list_index && m_list_index < (int)m_list.size()) {
		// expected
		token_t token = m_list.at(m_list_index);
		m_has_value   = true;
		m_value       = token_t(token, "/" + m_array_name + token.path());
		//
		m_list_index++;
		// special case for end of m_list
		if (m_list_index == (int)m_list.size()) {
			m_array_index++;
			m_list.clear();
		}
		return true;
	} else {
		logger.error("Unexpected m_list_index");
		logger.error("  m_list_index %4d", m_list_index);
		logger.error("  m_list.size  %4d", m_list.size());
		ERROR();
		return false;
	}
}
token_t json_expand_t::next_impl() {
	if (m_has_value) {
		m_has_value = false;
		return m_value;
	} else {
		// if there is no next and call next(), it is error
		logger.error("there is no next and call next() is error");
		ERROR();
		return m_value;
	}
}

#if 0
json_t parse(std::istream& in) {
	json_t json;
	json.parse(in);
	return json;
}
#endif


}
