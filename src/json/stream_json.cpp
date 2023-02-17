//
//
//

#include <thread>
#include <chrono>
#include <mutex>
#include <deque>
#include <regex>

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("stream_json");

#include "stream_util.h"
#include "stream_json.h"


constexpr auto glob_to_regex = json::glob_to_regex;


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
	auto impl = std::make_shared<json_impl_t>(in, max_queue_size, wait_time);
	return source_t<token_t>(impl, __func__);
}


//
// split
//
struct split_impl_t : public source_base_t<token_list_t> {
	using upstream_t = source_base_t<token_t>;

	upstream_t*   m_upstream;
	std::string  m_pattern;
	std::regex   m_regex;
	bool         m_has_value;
	token_list_t m_value;

	split_impl_t(upstream_t* upstream, std::string glob) :
		m_upstream(upstream),
		m_pattern(::json::glob_to_regex(glob)),
		m_regex(std::regex(m_pattern)),
		m_has_value(false) {}

	void close() override {}
	bool has_next() override {
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
	token_list_t next() override {
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
};
source_t<token_list_t> split(source_base_t<token_t>* upstream, const std::string& glob) {
	auto impl = std::make_shared<split_impl_t>(upstream, glob);
	return source_t<token_list_t>(impl, __func__);
}


//
// expand
//
struct expand_impl_t : public source_base_t<token_t> {
	using upstream_t = source_base_t<token_list_t>;

	upstream_t*   m_upstream;
	int           m_array_index      = 0;
	int           m_list_index       = 0;
	bool          m_has_value        = false;
	bool          m_need_first_array = true;
	bool          m_need_last_leave  = true;
	token_t       m_value;
	token_list_t  m_list;
	std::string   m_array_name;

	expand_impl_t(upstream_t* upstream) : m_upstream(upstream) {}

	void close() {}
	bool has_next() {
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
	token_t next() {
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
};
source_t<token_t> expand(source_base_t<token_list_t>* upstream) {
	auto impl = std::make_shared<expand_impl_t>(upstream);
	return source_t<token_t>(impl, __func__);
}


//
// include_path_value
//
struct include_path_value_predicate_t {
	std::regex regex_path;
	std::regex regex_value;

	include_path_value_predicate_t(const std::string& glob_path, const std::string& glob_value) :
		regex_path (glob_to_regex(glob_path)),
		regex_value(glob_to_regex(glob_value)) {}

	bool operator()(token_list_t list) const {
	    for(const auto& e: list) {
	    	if (std::regex_match(e.path(), regex_path) && std::regex_match(e.value(), regex_value)) return true;
	    }
	    return false;
	}
};
source_t<token_list_t> include_path_value(
	source_base_t<token_list_t>* upstream, const std::string& glob_path, const std::string& glob_value) {
	auto predicate(include_path_value_predicate_t(glob_path, glob_value));
	return stream::filter(upstream, predicate);
}


//
// exlude_path
//
struct exclude_path_predicate_t {
	std::regex m_regex;

	exclude_path_predicate_t(std::regex regex) : m_regex(regex) {}

	bool operator()(token_t token) const {
		// negate regex_match for exclude
		return !std::regex_match(token.path(), m_regex);
	}
};
source_t<token_t> exclude_path(source_base_t<token_t>* upstream, std::initializer_list<std::string> args) {
	assert(args.size() != 0);

	std::string string;
	for(auto e: args) {
		string.append("|(?:" + glob_to_regex(e) + ")");
	}
	std::regex regex = std::regex(string.substr(1));

	auto predicate = exclude_path_predicate_t(regex);
	return stream::filter(upstream, predicate);
}
source_t<token_t> exclude_path(source_base_t<token_t>* upstream, std::string glob_path) {
	std::regex regex = std::regex(glob_to_regex(glob_path));

	auto predicate = exclude_path_predicate_t(regex);
	return stream::filter(upstream, predicate);
}




// end of namespace
}
}
