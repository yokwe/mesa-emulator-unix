//
// stream.h
//

#pragma once

#include <string>

#include "../util/Util.h"

namespace stream {


template <typename T>
class iterator_t {
public:
	virtual ~iterator_t() {}

	virtual bool has_next() = 0;
	virtual T    next()     = 0;
};


class base_t {
public:
	base_t(const char* name) : m_name(name), m_closed(false), m_upstream(nullptr) {}
	base_t(const char* name, base_t* upstream) : m_name(name), m_closed(false), m_upstream(upstream) {}
	virtual ~base_t() {}

	virtual void close_impl() = 0; // most of case do nothing

	void close() {
		if (!m_closed) {
			m_closed = true;
			close_impl();
		}
	}

	const char* name() {
		return m_name;
	}
	base_t* upstream() {
		return m_upstream;
	}

protected:
	const char*    m_name;
	bool           m_closed;
	base_t*        m_upstream;
};


template <typename T>
class head_t : public base_t, public iterator_t<T> {
public:
	head_t(const char* name) : base_t(name) {}
	virtual ~head_t() {}

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

	bool has_next() override {
		return has_next_impl();
	}
	T next() override {
		if (has_next()) {
			return next_impl();
		} else {
			// if there is no next and call next(), it is error
			logger.error("stream has no next");
			ERROR();
		}
	}
};


template <typename T, typename R>
class tail_t : public base_t {
	iterator_t<T>* upstream;
public:
	tail_t(const char* name, base_t* upstream_) :
		base_t(name, upstream_),
		upstream(dynamic_cast<iterator_t<T>*>(upstream_)) {
		if (upstream == nullptr) {
			logger.error("upstream doesn't have iterator");
			logger.error("  myself    %s!", this->name());
			logger.error("  upstream_ %s!", upstream_->name());
			logger.error("  T         %s!", demangle(typeid(T).name()));
			logger.error("  R         %s!", demangle(typeid(R).name()));
			ERROR();
		}
	}
	virtual ~tail_t() {}

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

	R result() {
		return result_impl();
	}

	void accept(T& newValue) {
		logger.info("accept T&");
		accept_impl(newValue);
	}
	void accept(int newValue) {
		logger.info("accept int");
		accept_impl(newValue);
	}
	void accept(long newValue) {
		logger.info("accept long");
		accept_impl(newValue);
	}
	void accept(double newValue) {
		logger.info("accept double");
		accept_impl(newValue);
	}

	R process() {
		while(upstream->has_next()) {
			accept(upstream->next());
		}
		return result();
	}
};


template <typename T, typename R>
class map_t : public base_t, public iterator_t<R> {
	iterator_t<T>* upstream;
public:
	map_t(const char* name, base_t* upstream_) :
		base_t(name, upstream_),
		upstream(dynamic_cast<iterator_t<T>*>(upstream_)) {
		if (upstream == nullptr) {
			logger.error("upstream doesn't have iterator");
			logger.error("  myself    %s!", this->name());
			logger.error("  upstream_ %s!", upstream_->name());
			logger.error("  T         %s!", demangle(typeid(T).name()));
			logger.error("  R         %s!", demangle(typeid(R).name()));
			ERROR();
		}
	}
	virtual ~map_t() {}

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual R apply(T& newValue) = 0;

	bool has_next() override {
		return upstream->has_next();
	}
	R next() override {
		if (has_next()) {
			T newValue = upstream->next();
			return apply(newValue);
		} else {
			// if there is no next and call next(), it is error
			logger.error("stream has no next");
			ERROR();
		}
	}
};


}
