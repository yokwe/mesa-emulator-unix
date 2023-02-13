//
// stream.h
//

#pragma once

#include <initializer_list>
#include <tuple>

#include "../util/Util.h"

namespace stream {


class base_t {
	const char* m_name;
	bool        m_closed;

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
	const char* name() const {
		return m_name;
	}
};


template <typename T>
class source_t : public base_t {
	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual bool has_next_impl() = 0;
	virtual T    next_impl()     = 0;

public:
	const std::string type_T = demangle(typeid(T).name());

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


template <typename T, typename R>
class pipe_t : public source_t<R> {
protected:
	source_t<T>* m_upstream;

public:
	const std::string type_T = demangle(typeid(T).name());
	const std::string type_R = demangle(typeid(R).name());

	pipe_t(const char* name, source_t<T>* upstream) : source_t<R>(name), m_upstream(upstream) {}
	virtual ~pipe_t() {}
};


template <typename T, typename R>
class sink_t : public base_t {
	source_t<T>* m_upstream;

	// You need to implement base_t::close_impl() in implementing class
	// void close_impl();
	virtual void accept_impl(T& newValue) = 0;
	virtual R    result_impl()            = 0;

public:
	const std::string type_T = demangle(typeid(T).name());
	const std::string type_R = demangle(typeid(R).name());

	sink_t(const char* name, source_t<T>* upstream_) : base_t(name), m_upstream(upstream_) {}
	virtual ~sink_t() {}

	R result() {
		return result_impl();
	}

	void accept(T& newValue) {
		accept_impl(newValue);
	}
	void accept(int newValue) {
		accept_impl(newValue);
	}
	void accept(double newValue) {
		accept_impl(newValue);
	}

	R process() {
		while(m_upstream->has_next()) {
			T newValue = m_upstream->next();
			accept(newValue);
		}
		return result();
	}
};


//
// source_t
//
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


//
// sink_t
//
template <typename T, typename R=T>
class sum_t : public sink_t<T, R> {
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,  "T is not number");
	static_assert(std::is_integral_v<R> || std::is_floating_point_v<R>,  "R is not number");

	R m_sum;

	void close_impl() override {}
	void accept_impl(T& newValue) override {
		m_sum += newValue;
	}
	R result_impl() override {
		return m_sum;
	}

public:
	sum_t(source_t<T>* upstream) : sink_t<T, R>(__func__, upstream), m_sum(0) {}
	~sum_t() {
		base_t::close();
	}
};

template <typename T, typename R=int>
class count_sink_t : public sink_t<T, R> {
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
	count_sink_t(source_t<T>* upstream) : sink_t<T, R>(__func__, upstream), m_count(0) {}
	~count_sink_t() {
		base_t::close();
	}
};


//
// pipe_t
//
template <typename T, typename R=T>
class peek_t : public pipe_t<T, R> {
	using peek_apply = std::function<void(T)>;
	peek_apply   m_apply;

	void close_impl() override {}
	bool has_next_impl() override {
		return pipe_t<T, R>::m_upstream->has_next();
	}
	R next_impl() override {
		R newValue = pipe_t<T, R>::m_upstream->next();
		m_apply(newValue);
		return newValue;
	}

public:
	peek_t(source_t<T>* upstream, peek_apply apply) :
		pipe_t<T, R>(__func__, upstream),
		m_apply(apply) {}
	~peek_t() {
		base_t::close();
	}
};


template <typename T, typename R=T>
class tee_t : public pipe_t<T, R> {
public:
	class callback_t {
	public:
		virtual ~callback_t() {}
		// life cycle event
		virtual void start() = 0;
		virtual void stop() = 0;
		// data event
		virtual void data(T& t) = 0;
	};
	callback_t& m_callback;

	tee_t(source_t<T>* upstream, callback_t& callback_) :
		pipe_t<T, R>(__func__, upstream),
		m_callback(callback_) {
		m_callback.start();
	}
	~tee_t() {
		base_t::close();
	}

	void close_impl() override {
		m_callback.stop();
	}
	bool has_next_impl() override {
		return pipe_t<T, R>::m_upstream->has_next();
	}
	R next_impl() override {
		R newValue = pipe_t<T, R>::m_upstream->next();
		m_callback.data(newValue);
		return newValue;
	}
};


template <typename T, typename R>
class map_t : public pipe_t<T, R> {
	using map_apply = std::function<R(T)>;

	map_apply    m_apply;

	void close_impl() override {}
	bool has_next_impl() override {
		return pipe_t<T, R>::m_upstream->has_next();
	}
	R next_impl() override {
		T newValue = pipe_t<T, R>::m_upstream->next();
		return m_apply(newValue);
	}

public:
	map_t(source_t<T>* upstream, map_apply apply) :
		pipe_t<T, R>(__func__, upstream),
		m_apply(apply) {}
	~map_t() {
		base_t::close();
	}
};


template <typename T>
class filter_t : public pipe_t<T, T> {
	using filter_test = std::function<bool(T)>;

	filter_test  m_test;
	bool         m_has_value;
	T            m_value;

	void close_impl() override {}
	bool has_next_impl() override {
		for(;;) {
			if (m_has_value) break;
			if (!pipe_t<T, T>::m_upstream->has_next()) break;
			m_value = pipe_t<T, T>::m_upstream->next();
			m_has_value = m_test(m_value);
		}
		return m_has_value;
	}
	T next_impl() override {
		if (has_next_impl()) {
			m_has_value = false;
			return m_value;
		} else {
			// if there is no next and call next(), it is error
			logger.error("stream has no next");
			ERROR();
		}
	}

public:
	filter_t(source_t<T>* upstream, filter_test test) :
		pipe_t<T, T>(__func__, upstream),
		m_test(test),
		m_has_value(false) {}
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

//
// source
//
template <typename T>
vector_t<T> vector(std::initializer_list<T> init) {
	return vector_t<T>(init);
}

//
// pipe
//
template <typename T>
auto tee(source_t<T>* upstream, typename tee_t<T>::callback_t& callback_) {
	return tee_t<T>(upstream, callback_);
}

template <typename T, typename Function>
auto peek(source_t<T>* upstream,  Function apply) {
	using trait = trait_function<Function>;
	using R   = typename trait::ret_type;
	using A0_ = typename std::tuple_element<0, typename trait::arg_type>;
	using A0  = typename A0_::type;

	//logger.debug("map T=%s R=%s Function=%s", demangle(typeid(T).name()), demangle(typeid(R).name()), demangle(typeid(Function).name()));
	static_assert(trait::arity == 1);
	static_assert(std::is_same_v<T, A0>);

	// assert T     == A0T
	// assert arity == 1
	if constexpr (!std::is_same_v<T, A0> || !(trait::arity == 1) || !std::is_same_v<R, void>) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("arity %d", trait::arity);
		logger.error("R     %s", demangle(typeid(R).name()));
		logger.error("T     %s", demangle(typeid(T).name()));
		logger.error("A0    %s", demangle(typeid(A0).name()));
		ERROR();
	}

	return peek_t<T>(upstream, apply);
}

template <typename T, typename Function>
auto map(source_t<T>* upstream,  Function apply) {
	using trait = trait_function<Function>;
	using R   = typename trait::ret_type;
	using A0_ = typename std::tuple_element<0, typename trait::arg_type>;
	using A0  = typename A0_::type;

	static_assert(trait::arity == 1);
	static_assert(std::is_same_v<T, A0>);

	// assert T     == A0T
	// assert arity == 1
	if constexpr (!std::is_same_v<T, A0> || !(trait::arity == 1)) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("arity %d", trait::arity);
		logger.error("R     %s", demangle(typeid(R).name()));
		logger.error("T     %s", demangle(typeid(T).name()));
		logger.error("A0    %s", demangle(typeid(A0).name()));
		ERROR();
	}

	return map_t<T, R>(upstream, apply);
}
template <typename T, typename Predicate>
filter_t<T>	filter(source_t<T>* upstream, Predicate test) {
	using trait = trait_function<Predicate>;
	using R   = typename trait::ret_type;
	using A0_ = typename std::tuple_element<0, typename trait::arg_type>;
	using A0  = typename A0_::type;

	//logger.debug("filter T=%s R=%s Predicater=%s", demangle(typeid(T).name()), demangle(typeid(R).name()), demangle(typeid(Predicate).name()));
	static_assert(trait::arity == 1);
	static_assert(std::is_same_v<T, A0>);
	static_assert(std::is_same_v<R, bool>);

	// assert T     == A0T
	// assert R     == bool
	// assert arity == 1
	if constexpr (!std::is_same_v<T, A0> || !std::is_same_v<R, bool> || !(trait::arity == 1)) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("arity %d", trait::arity);
		logger.error("R     %s", demangle(typeid(R).name()));
		logger.error("T     %s", demangle(typeid(T).name()));
		logger.error("A0    %s", demangle(typeid(A0_).name()));
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

template<typename T, typename R=int>
count_sink_t<T, R> count(source_t<T>* upstream) {
	return count_sink_t<T, R>(upstream);
}


}
