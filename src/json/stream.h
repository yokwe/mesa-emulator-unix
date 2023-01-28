//
// stream.h
//

#pragma once

#include "../util/Util.h"

namespace stream {

template <typename T>
class iterator_t {
public:
	virtual ~iterator_t() {}

	virtual bool has_next() = 0;
	virtual T    next()     = 0;
};

enum type_t {
	HEAD, BODY, TAIL,
};


class base_t {
public:
	base_t(type_t type, const char* name) :
		m_type(type), m_name(name), m_closed(false) {}
	virtual ~base_t() {}

	void close_impl() {} // do nothing

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
	bool           m_closed;
};


template <typename T>
class head_t : public base_t, public iterator_t<T> {
public:
	head_t(const char* name) : base_t(type_t::HEAD, name) {}
	virtual ~head_t() {}

	// need to implement virtual method of parent classes
	//   base_t   close_impl()

	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

	bool has_next() override {
		if (base_t::closed()) {
			return false;
		} else {
			bool ret = has_next_impl();
			// if no more data, call close
			if (!ret) base_t::close();
			return ret;
		}
	}
	T next() override {
		if (base_t::closed()) {
			logger.error("logger is already closed");
			ERROR();
		} else {
			return next_impl();
		}
	}
};


template <typename T, typename R>
class tail_t : public base_t {
	iterator_t<T>* m_iterator;
public:
	tail_t(const char* name, base_t* previous) :
		base_t(type_t::TAIL, name),
		m_iterator(dynamic_cast<iterator_t<T>*>(previous)) {}
	virtual ~tail_t() {}

	// need to implement virtual method of parent classes
	//   base_t   close_impl()

	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

	R result() {
		if (base_t::closed()) {
			return result_impl();
		} else {
			ERROR();
		}
	}

	void accept(T& newValue) {
		if (base_t::closed()) {
			ERROR();
		} else {
			accept_impl(newValue);
		}
	}

	R process() {
		while(m_iterator->has_next()) {
			T t = m_iterator->next();
			accept(t);
		}
		base_t::close();
		return result();
	}
};


// 1 to 1
template <typename T, typename R>
class body11_t : public base_t, public iterator_t<R> {
	iterator_t<T>* m_iterator;
public:
	body11_t(const char* name, base_t* previous) :
		base_t(type_t::BODY, name),
		m_iterator(dynamic_cast<iterator_t<T>*>(previous)) {}
	virtual ~body11_t() {}

	virtual R apply(T& newValue) = 0;

	bool has_next() {
		return m_iterator->has_next();
	}
	R next() {
		T newValue = m_iterator->next();
		return apply(newValue);
	}
};


}
