//
// stream.h
//

#pragma once

#include <type_traits>
#include <tuple>
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
	const std::string data_type_name = demangle(typeid(T).name());

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
	std::vector<T> m_data;
	std::size_t    m_pos;

	void close_impl() override {}
	bool has_next_impl() override {
		return 0 <= m_pos && m_pos < m_data.size();
	}
	T next_impl() override {
		return m_data[m_pos++];
	}

public:
	vector_t(std::vector<T>& data) :
		source_t<T>(__func__), m_data(data),
		m_pos(0) {}
	vector_t(std::initializer_list<T> init) :
		source_t<T>(__func__), m_data(init.begin(), init.end()),
		m_pos(0) {}
	~vector_t() {
		base_t::close();
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
	void accept(unsigned int newValue) {
		accept_impl(newValue);
	}
	void accept(unsigned long newValue) {
		accept_impl(newValue);
	}
	void accept(unsigned long long newValue) {
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
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,  "T is not number");
	static_assert(std::is_integral_v<R> || std::is_floating_point_v<R>,  "R is not number");

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


template <typename T, typename R=T>
class map_t : public source_t<R> {
	using map_apply = std::function<R(T)>;

	source_t<T>* upstream;
	map_apply    apply;

	void close_impl() override {}
	bool has_next_impl() override {
		return upstream->has_next();
	}
	R next_impl() override {
		T newValue = upstream->next();
		return apply(newValue);
	}

public:
	map_t(source_t<T>* upstream_, map_apply apply_) :
		source_t<R>(__func__),
		upstream(upstream_),
		apply(apply_) {}
	~map_t() {
		base_t::close();
	}
};


template <typename T>
class filter_t : public source_t<T> {
	using filter_test = std::function<bool(T)>;

	source_t<T>* upstream;
	filter_test  test;
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
	filter_t(source_t<T>* upstream_, filter_test test_) :
		source_t<T>(__func__),
		upstream(upstream_),
		test(test_),
		hasValue(false) {}
	~filter_t() {
		base_t::close();
	}
};


template <typename T>
class count_t : public source_t<T> {
	source_t<T>* upstream;
	int          count;

	void close_impl() override {}
	bool has_next_impl() override {
		return upstream->has_next();
	}
	T next_impl() override {
		count++;
		return upstream->next();
	}

public:
	count_t(source_t<T>* upstream_, const char* name_) :
		source_t<T>(name_),
		upstream(upstream_),
		count(false) {}
	~count_t() {
		base_t::close();
		logger.info("count_t %s %d", base_t::name(), count);
	}
};


//
// function
//
template <typename T>
struct trait_function : trait_function<decltype(&T::operator())> {};
template <typename C, typename R, typename... A>
struct trait_function<R(C::*)(A...) const> {
	enum {type = 100};

	// use enum to define type not constant
	enum {arity = sizeof...(A)};
	// return type
	using ret_type = R;
	// argument type as std::tuple<>
	using arg_type = std::tuple<A...>;
};
template <typename R, typename... A>
struct trait_function<R(*)(A...)> {
	enum {type = 200};

	// use enum to define type not constant
	enum {arity = sizeof...(A)};
	// return type
	using ret_type = R;
	// argument type as std::tuple<>
	using arg_type = std::tuple<A...>;
};
#if 0
using trait = trait_function<T>;
logger.info("trait    %s", demangle(typeid(trait)));
logger.info("type     %d", trait::type);
logger.info("arity    %d", trait::arity);
logger.info("ret_type %s", demangle(typeid(typename trait::ret_type)));
using args0 = typename std::tuple_element<0, typename trait::arg_type>;
logger.info("arg0     %s", demangle(typeid(typename args0::type)));
#endif


//
// source
//
template <typename T>
vector_t<T> vector(std::initializer_list<T> init) {
	return vector_t<T>(init);
}

//
// between
//
template <typename T, typename Function>
auto map(source_t<T>* upstream,  Function apply) {
	using trait = trait_function<Function>;
	using R   = typename trait::ret_type;
	using A0  = typename std::tuple_element<0, typename trait::arg_type>;
	using A0T = typename A0::type;

	//logger.debug("map T=%s R=%s Function=%s", demangle(typeid(T).name()), demangle(typeid(R).name()), demangle(typeid(Function).name()));

	// assert T == A0T
	if constexpr (!std::is_same_v<T, A0T>) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("T != A0T");
		logger.error("T  %s", demangle(typeid(T).name()));
		logger.error("A0 %s", demangle(typeid(A0T).name()));
		ERROR();
	}

	return map_t<T, R>(upstream, apply);
}
template <typename T, typename Predicate>
filter_t<T>	filter(source_t<T>* upstream, Predicate test) {
	using trait = trait_function<Predicate>;
	using R   = typename trait::ret_type;
	using A0  = typename std::tuple_element<0, typename trait::arg_type>;
	using A0T = typename A0::type;

	//logger.debug("filter T=%s R=%s Predicater=%s", demangle(typeid(T).name()), demangle(typeid(R).name()), demangle(typeid(Predicate).name()));

	// assert T == A0T
	if constexpr (!std::is_same_v<T, A0T>) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("T != A0T");
		logger.error("T  %s", demangle(typeid(T).name()));
		logger.error("A0 %s", demangle(typeid(A0T).name()));
		assert(false);
	}

	// assrt R == bool
	if constexpr (!std::is_same_v<R, bool>) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("R != bool");
		logger.error("R  %s", demangle(typeid(R).name()));
		assert(false);
	}

	return filter_t<T>(upstream, test);
}
template <typename T>
count_t<T>	count(source_t<T>* upstream, const char* name) {
	return count_t<T>(upstream, name);
}

//
// sink
//
template<typename T, typename R=T>
sum_t<T, R> sum(source_t<T>* upstream) {
	return sum_t<T, R>(upstream);
}

}

#if 0
{
	auto head   = stream::vector({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
	auto countA = stream::count(&head, "countA");
	auto filter = stream::filter(&countA, [](int a){return a < 6;});
	auto add    = stream::map(&filter,    [](int a){return a + 1000;});
	auto sub    = stream::map(&add,       [](int a){return a - 1000;});
	auto countB = stream::count(&sub, "countB");
	auto sum    = stream::sum(&countB);

	logger.info("sum %s", std::to_string(sum.process()));
}
#endif
