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

private:
	const char*    m_name;
	bool           m_closed;
	base_t*        m_upstream;
};


template <typename T>
class head_t : public base_t, public iterator_t<T> {
	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

public:
	head_t(const char* name) : base_t(name) {}
	virtual ~head_t() {}

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


template <typename T>
class vector_t : public head_t<T> {
	::std::vector<T> m_data;
	::std::size_t    m_pos;
public:
	vector_t(::std::vector<T>& data) :
		head_t<T>(__func__), m_data(data),
		m_pos(0) {}
	vector_t(::std::initializer_list<T> init) :
		head_t<T>(__func__), m_data(init.begin(), init.end()),
		m_pos(0) {}
	~vector_t() {
		base_t::close();
	}
private:
	void close_impl() {}
	bool has_next_impl() {
		return 0 <= m_pos && m_pos < m_data.size();
	}
	T next_impl() {
		return m_data[m_pos++];
	}
};


template <typename T, typename R>
class tail_t : public base_t {
	iterator_t<T>* upstream;

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

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

	R result() {
		return result_impl();
	}

	void accept(T& newValue) {
		logger.info("accept T&");
		accept_impl(newValue);
	}
	void accept(int newValue) {
		accept_impl(newValue);
	}
	void accept(long newValue) {
		accept_impl(newValue);
	}
	void accept(long long newValue) {
		accept_impl(newValue);
	}
	void accept(double newValue) {
		accept_impl(newValue);
	}

	R process() {
		while(upstream->has_next()) {
			accept(upstream->next());
		}
		return result();
	}
};


template <typename T, typename R=T>
class sum_t : public tail_t<T, R> {
	static_assert(::std::is_integral<T>::value || ::std::is_floating_point<T>::value,  "T is not number");
	static_assert(::std::is_integral<R>::value || ::std::is_floating_point<R>::value,  "R is not number");

	R sum;

	void close_impl() override {}
	void accept_impl(T& newValue) override {
		sum += newValue;
	}
	R result_impl() override {
		return sum;
	}
public:
	sum_t(base_t* upstream) : tail_t<T, R>(__func__, upstream), sum(0) {}
	~sum_t() {
		base_t::close();
	}
};


template <typename T, typename R=int>
class count_t : public tail_t<T, R> {
	static_assert(::std::is_integral<R>::value || ::std::is_floating_point<R>::value,  "R is not number");

	R m_count;

	void close_impl() override {}
	void accept_impl(T& newValue) override {
		(void)newValue;
		m_count++;
	}
	R result_impl() override {
		return m_count;
	}
public:
	count_t(base_t* upstream) : tail_t<T, R>(__func__, upstream), m_count(0) {}
	~count_t() {
		base_t::close();
	}
};


template <typename T, typename R=T>
class map_abstract_t : public base_t, public iterator_t<R> {
	iterator_t<T>* upstream;

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual R apply(T& newValue) = 0;

public:
	map_abstract_t(const char* name, base_t* upstream_) :
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
	virtual ~map_abstract_t() {}

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


template <typename T, typename R=T>
class map_t : public base_t, public iterator_t<R> {
	typedef ::std::function<R(T)> func_apply;
	iterator_t<T>* upstream;
	func_apply     apply;

	void close_impl() override {}

public:
	map_t(base_t* upstream_, func_apply apply_) :
		base_t(__func__, upstream_),
		upstream(dynamic_cast<iterator_t<T>*>(upstream_)),
		apply(apply_) {
		if (upstream == nullptr) {
			logger.error("upstream doesn't have iterator");
			logger.error("  myself    %s!", this->name());
			logger.error("  upstream_ %s!", upstream_->name());
			logger.error("  T         %s!", demangle(typeid(T).name()));
			logger.error("  R         %s!", demangle(typeid(R).name()));
			ERROR();
		}
	}
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


template <typename T>
vector_t<T> vector(::std::initializer_list<T> init) {
	return vector_t<T>(init);
}

template<typename S, typename T, typename R=T>
sum_t<T, R> sum(map_t<S, T>* upstream) {
	return sum_t<T, R>(upstream);
}

template <typename T, typename R>
map_t<T, R> map(base_t* upstream, ::std::function<R(T)> func) {
	return map_t<T, R>(upstream, func);
}


}


//{
//	::std::function<int(int)> func_add = [](int a){return a + 1000;};
//	::std::function<int(int)> func_sub = [](int a){return a - 1000;};
//
//	auto head = stream::vector({1, 2, 3, 4});
//	auto add  = stream::map(&head, func_add);
//	auto sub  = stream::map(&add,  func_sub);
//	auto sum  = stream::sum(&sub);
//
//	logger.info("sum %s", std::to_string(sum.process()));
//}
