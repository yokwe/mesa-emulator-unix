//
// stream.h
//

#pragma once

#include <string>

#include "../util/Util.h"

namespace stream {


class base_t {
public:
	base_t(const char* name) : m_name(name), m_closed(false) {}
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

private:
	const char* m_name;
	bool        m_closed;
};


template <typename T>
class source_t : public base_t {
	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

public:
	const ::std::string data_type_name = demangle(typeid(T).name());

	source_t(const char* name) : base_t(name) {}

	virtual ~source_t() {}

	bool has_next() {
		return has_next_impl();
	}
	T next() {
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
class vector_t : public source_t<T> {
	::std::vector<T> m_data;
	::std::size_t    m_pos;
public:
	vector_t(::std::vector<T>& data) :
		source_t<T>(__func__), m_data(data),
		m_pos(0) {}
	vector_t(::std::initializer_list<T> init) :
		source_t<T>(__func__), m_data(init.begin(), init.end()),
		m_pos(0) {}
	~vector_t() {
		base_t::close();
	}
private:
	void close_impl() override {}
	bool has_next_impl() override {
		return 0 <= m_pos && m_pos < m_data.size();
	}
	T next_impl() override {
		return m_data[m_pos++];
	}
};


template <typename T, typename R>
class sink_t : public base_t {
	source_t<T>* upstream;

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

public:
	sink_t(const char* name, source_t<T>* upstream_) :
		base_t(name),
		upstream(upstream_) {
	}
	virtual ~sink_t() {}

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
class sum_t : public sink_t<T, R> {
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
	sum_t(source_t<T>* upstream) : sink_t<T, R>(__func__, upstream), sum(0) {}
	~sum_t() {
		base_t::close();
	}
};


template <typename T, typename R=int>
class count_t : public sink_t<T, R> {
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
	count_t(base_t* upstream) : sink_t<T, R>(__func__, upstream), m_count(0) {}
	~count_t() {
		base_t::close();
	}
};

template <typename T, typename R>
using map_apply = ::std::function<R(T)>;

template <typename T, typename R=T>
class map_t : public source_t<R> {
	source_t<T>*    upstream;
	map_apply<T, R> apply;

	void close_impl() override {}
	bool has_next_impl() override {
		return upstream->has_next();
	}
	R next_impl() override {
		T newValue = upstream->next();
		return apply(newValue);
	}
public:
	map_t(source_t<T>* upstream_, map_apply<T, R> apply_) :
		source_t<T>(__func__),
		upstream(upstream_),
		apply(apply_) {}
};


template <typename T>
using filter_test = ::std::function<bool(T)>;

template <typename T>
class filter_t : public source_t<T> {
	source_t<T>* upstream;
	filter_test<T>    test;
	bool         hasValue;
	T            value;

	void close_impl() override {}
	bool has_next_impl() override {
		for(;;) {
			if (hasValue) break;
			if (!upstream->has_next()) break;
			value = upstream->next();
			hasValue = test(value);
		}
		return hasValue;
	}
	T next_impl() override {
		if (has_next_impl()) {
			hasValue = false;
			return value;
		} else {
			// if there is no next and call next(), it is error
			logger.error("stream has no next");
			ERROR();
		}
	}

public:
	filter_t(source_t<T>* upstream_, filter_test<T> test_) :
		source_t<T>(__func__),
		upstream(upstream_),
		test(test_),
		hasValue(false) {}
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
map_t<T, R> map(source_t<T>* upstream, map_apply<T, R> func) {
	return map_t<T, R>(upstream, func);
}

template <typename T>
filter_t<T>	filter(source_t<T>* upstream, filter_test<T> func) {
	return filter_t<T>(upstream, func);
}

}

#if 0
{
	stream::map_apply<int, int> func_add    = [](int a){return a + 1000;};
	stream::map_apply<int, int> func_sub    = [](int a){return a - 1000;};
	stream::filter_test<int>    func_filter = [](int a){return a < 3;};

	auto head   = stream::vector({1, 2, 3, 4});
	auto filter = stream::filter(&head, func_filter);
	auto add    = stream::map(&filter, func_add);
	auto sub    = stream::map(&add,  func_sub);
	auto sum    = stream::sum(&sub);

	logger.info("sum %s", std::to_string(sum.process()));
}
#endif
