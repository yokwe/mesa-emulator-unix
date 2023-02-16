//
//
//

#pragma once

#include <string>

#include "../util/Util.h"

#include "stream.h"

namespace stream {
// begin of namespace stream


//
// source  vector
//
template <typename T>
struct vector_impl_t : source_base_t<T> {
	std::vector<T> m_data;
	std::size_t    m_pos    = 0;

	void close() override {}
	bool has_next() override {
		return 0 <= m_pos && m_pos < m_data.size();
	}
	T next() override {
		return m_data[m_pos++];
	}

	vector_impl_t(std::vector<T>& data) : m_data(data) {}
	vector_impl_t(std::initializer_list<T> init) :m_data(init.begin(), init.end()) {}
};
template <typename T>
source_t<T> vector(std::initializer_list<T> init) {
	auto impl = std::make_shared<vector_impl_t<T>>(init);
	return source_t<T>(impl, __func__);
}
template <typename T>
source_t<T> vector(std::vector<T>& data) {
	auto impl = std::make_shared<vector_impl_t<T>>(data);
	return source_t<T>(impl, __func__);
}


//
// sink  sum
//
template <typename T, typename R>
struct sum_impl_t : public sink_t<T, R>::base_t {
	R m_sum = 0;

	void close() override {}
	void accept(T& newValue) override {
		m_sum += newValue;
	}
	R result() override {
		return m_sum;
	}
};
template<typename T, typename R=T>
sink_t<T, R> sum(source_base_t<T>* upstream) {
	auto impl = std::make_shared<sum_impl_t<T, R>>();
	return sink_t<T, R>(impl, __func__, upstream);
}


//
// sink  count
//
template <typename T, typename R>
struct sink_count_impl_t : public sink_t<T, R>::base_t {
	R m_count = 0;

	void close() override {}
	void accept(T& newValue) override {
		(void)newValue;
		m_count++;
	}
	R result() override {
		return m_count;
	}
};
template <typename T, typename R=int>
sink_t<T, R> count(source_base_t<T>* upstream) {
	auto impl = std::make_shared<sink_count_impl_t<T, R>>();
	return sink_t<T, R>(impl, __func__, upstream);
}


//
// source count
//
template <typename T>
struct source_count_impl_t : public source_base_t<T> {
	using upstream_t = source_base_t<T>;

	upstream_t* m_upstream;
	std::string m_name;
	int         m_count = 0;

	source_count_impl_t(upstream_t* upstream, const std::string& name) : m_upstream(upstream), m_name(name) {}

	void close() override {
		logger.info("count %s %d", m_name, m_count);
	}
	bool has_next() override {
		return m_upstream->has_next();
	}
	T next() override {
		m_count++;
		T newValue = m_upstream->next();
		return newValue;
	}
};
template <typename T>
source_t<T> count(source_base_t<T>* upstream, const std::string& name) {
	auto impl = std::make_shared<source_count_impl_t<T>>(upstream, name);
	return source_t<T>(impl, __func__);
}
template <typename T>
source_t<T> count(source_base_t<T>* upstream, const char* name) {
	auto impl = std::make_shared<source_count_impl_t<T>>(upstream, std::string(name));
	return source_t<T>(impl, __func__);
}


//
// sink null
//
template <typename T, typename R>
struct sink_null_impl_t : public sink_t<T, R>::base_t {
	void close()    override {}
	void accept(T&) override {}
	R    result()   override {}
};
template <typename T>
sink_t<T, void> null(source_base_t<T>* upstream) {
	auto impl = std::make_shared<sink_null_impl_t<T, void>>();
	return sink_t<T, void>(impl, __func__, upstream);
}


//
// source map
//
// R Function()(T)
template <typename T, typename R, typename Function>
struct map_impl_t : public source_base_t<R> {
	using upstream_t = source_base_t<T>;

	upstream_t* m_upstream;
	Function    m_function;

	map_impl_t(upstream_t* upstream_, Function function_) : m_upstream(upstream_), m_function(function_) {}

	void close() override {}
	bool has_next() override {
		return m_upstream->has_next();
	}
	R next() override {
		T newValue = m_upstream->next();
		return m_function(newValue);
	}
};
template <typename T, typename Function>
auto map(source_base_t<T>* upstream,  Function apply) {
	using trait = trait_function<Function>;
	using R   = typename trait::ret_type;
	using A0_ = typename std::tuple_element<0, typename trait::arg_type>;
	using A0  = typename A0_::type;

	static_assert(trait::arity == 1);
	static_assert(std::is_same_v<T, A0>);

	// assert T     == A0T
	// assert arity == 1
//	if constexpr (!std::is_same_v<T, A0> || !(trait::arity == 1)) {
//		logger.error("ASSERTION FAILED");
//		logger.error("function %s", __func__);
//		logger.error("arity %d", trait::arity);
//		logger.error("R     %s", demangle(typeid(R).name()));
//		logger.error("T     %s", demangle(typeid(T).name()));
//		logger.error("A0    %s", demangle(typeid(A0).name()));
//		ERROR();
//	}

	auto impl = std::make_shared<map_impl_t<T, R, Function>>(upstream, apply);
	return source_t<R>(impl, __func__);
}


//
// source filter
//
// bool Predicate()(T)
template <typename T, typename Predicate>
struct filter_impl_t : public source_base_t<T> {
	using upstream_t = source_base_t<T>;

	upstream_t* m_upstream;
	Predicate   m_predicate;
	bool        m_has_value = false;
	T           m_value;

	filter_impl_t(upstream_t* upstream, Predicate predicate) : m_upstream(upstream), m_predicate(predicate) {}

	void close() override {}
	bool has_next() override {
		for(;;) {
			if (m_has_value) break;
			if (!m_upstream->has_next()) break;
			m_value = m_upstream->next();
			m_has_value = m_predicate(m_value);
		}
		return m_has_value;
	}
	T next() override {
		if (has_next()) {
			m_has_value = false;
			return m_value;
		} else {
			// if there is no next and call next(), it is error
			logger.error("stream has no next");
			ERROR();
		}
	}
};
template <typename T, typename Predicate>
source_t<T>	filter(source_base_t<T>* upstream, Predicate predicate) {
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

	auto impl = std::make_shared<filter_impl_t<T, Predicate>>(upstream, predicate);
	return source_t<T>(impl, __func__);

}


//
// source peek
//
// void Consumer()(T)
template <typename T, typename Consumer>
struct peek_impl_t : public source_base_t<T> {
	using upstream_t  = source_base_t<T>;

	upstream_t* m_upstream;
	Consumer    m_consumer;

	peek_impl_t(upstream_t* upstream, Consumer consumer) : m_upstream(upstream), m_consumer(consumer) {}

	void close() override {}
	bool has_next() override {
		return m_upstream->has_next();
	}
	T next() override {
		T newValue = m_upstream->next();
		m_consumer(newValue);
		return newValue;
	}
};
template <typename T, typename Consumer>
source_t<T>	peek(source_base_t<T>* upstream, Consumer consumer) {
	auto impl = std::make_shared<peek_impl_t<T, Consumer>>(upstream, consumer);
	return source_t<T>(impl, __func__);
}


// end of namespace stream
}
