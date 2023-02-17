//
// stream.h
//

#pragma once

#include <string>

#include "../util/Util.h"

namespace stream {
// begin of namespace stream


template <typename T>
struct source_base_t {
	virtual ~source_base_t() {}

	virtual void close()    = 0;
	virtual bool has_next() = 0;
	virtual T    next()     = 0;
};


//
// source_t
//
template <typename T>
class source_t : public source_base_t<T> {
private:
	using base_t = source_base_t<T>;
	using impl_t = std::shared_ptr<base_t>;

	impl_t      m_impl;
	std::string m_name;
	bool        m_closed = false;
public:
	source_t(impl_t impl, const char* name) : m_impl(impl),  m_name(name) {}
	~source_t() {
		close();
	}
	std::string name() {
		return m_name;
	}

	void close() override {
		if (!m_closed) {
			m_closed = true;
			m_impl->close();
		}
	}
	bool has_next() override {
		if (m_closed) {
			return false;
		} else {
			return m_impl->has_next();
		}
	}
	T next() override {
		if (has_next()) {
			return m_impl->next();
		} else {
			// if there is no next and call next(), it is error
			assert(false);
		}
	}
};


//
// sink_t
//
template <typename T, typename R>
class sink_t {
public:
	using upstream_t = source_base_t<T>;

	struct base_t {
		virtual ~base_t() {}

		virtual void close()      = 0;
		virtual void accept(T& t) = 0;
		virtual R    result()     = 0;
	};
	using impl_t = std::shared_ptr<base_t>;

private:
	impl_t       m_impl;
	std::string  m_name;
	upstream_t*  m_upstream;
	bool         m_closed = false;
public:
	sink_t(impl_t impl, const char* name, source_base_t<T>* upstream_) : m_impl(impl),  m_name(name), m_upstream(upstream_) {}
	virtual ~sink_t() {
		close();
	}
	std::string name() {
		return m_name;
	}

	void close() {
		if (!m_closed) {
			m_closed = true;
			m_impl->close();
		}
	}
	R result() {
		return m_impl->result();
	}

	void accept(T& newValue) {
		m_impl->accept(newValue);
	}

	R process() {
		while(m_upstream->has_next()) {
			T newValue = m_upstream->next();
			accept(newValue);
		}
		return result();
	}
};


// end of namespace stream
}
