//
// stream.h
//

#pragma once

#include "../util/Util.h"

namespace stream {

template <typename T>
class java_iterator_t {
public:
	virtual ~java_iterator_t() {}

	virtual bool has_next() = 0;
	virtual T    next()     = 0;
};

enum type_t {
	HEAD, BODY, TAIL,
};

class base_t;
class base_t {
public:
	base_t(type_t type, const char* name, base_t* previous) :
		m_type(type), m_name(name), m_previous(previous), m_closed(false) {}
	virtual ~base_t() {}

	virtual void close_impl() = 0;

	void close() {
		if (!closed()) {
			m_closed = true;
			close_impl();
		}
	}

	type_t type() {
		return m_type;
	}
	const char* name() {
		return m_name;
	}
	bool closed() {
		return m_closed;
	}

protected:
	type_t         m_type;
	const char*    m_name;
	base_t*        m_previous;
	bool           m_closed;
};


template <typename T>
class head_t : public base_t, public java_iterator_t<T> {
public:
	head_t(const char* name) : base_t(type_t::HEAD, name, nullptr) {
		assert(m_previous == nullptr);
	}
	head_t(type_t type, const char* name, base_t* previous) : base_t(type, name, previous) {
		assert(m_previous != nullptr);
	}
	virtual ~head_t() {}

	// need to implement virtual method of parent classes
	//   base_t   close_impl()

	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

	bool has_next() override {
		if (m_closed) {
			return false;
		} else {
			bool ret = has_next_impl();
			// if no more data, call close
			if (!ret) close();
			return ret;
		}
	}
	T next() override {
		if (m_closed) {
			logger.error("logger is already closed");
			ERROR();
		} else {
			return next_impl();
		}
	}
};


template <typename T, typename R>
class tail_t : public base_t {
	head_t<T>* m_head;
public:
	tail_t(const char* name, base_t* previous) : base_t(type_t::TAIL, name, previous) {
		m_head = dynamic_cast<head_t<T>*>(previous);
		assert(m_head != nullptr);
	}
	virtual ~tail_t() {}

	// need to implement virtual method of parent classes
	//   base_t   close_impl()

	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

	R result() {
		if (closed()) {
			return result_impl();
		} else {
			ERROR();
		}
	}

	void accept(T& newValue) {
		if (closed()) {
			ERROR();
		} else {
			accept_impl(newValue);
		}
	}

	R process() {
		while(m_head->has_next()) {
			auto t = m_head->next();
			accept(t);
		}
		close();
		return result();
	}
};


}
