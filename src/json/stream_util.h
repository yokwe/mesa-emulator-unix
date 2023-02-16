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
	auto impl = std::make_shared<sink_t<T, R>>();
	return sink_t<T, R>(impl, __func__, upstream);
}


//
// sink  count
//
template <typename T, typename R>
struct count_impl_t : public sink_t<T, R>::base_t {
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
template<typename T, typename R=int>
sink_t<T, R> count(source_base_t<T>* upstream) {
	auto impl = std::make_shared<count_impl_t<T, R>>();
	return sink_t<T, R>(impl, __func__, upstream);
}


// pipe map
template <typename T, typename R>
struct map_impl_t : public source_base_t<R> {
	using upstream_t = source_base_t<T>;
	using map_apply  = std::function<R(T)>;

	upstream_t* m_upstream;
	map_apply   m_apply;

	map_impl_t(upstream_t* upstream_, map_apply apply_) : m_upstream(upstream_), m_apply(apply_) {}

	void close() override {}
	bool has_next() override {
		return m_upstream->has_next();
	}
	R next() override {
		T newValue = m_upstream->next();
		return m_apply(newValue);
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

	auto impl = std::make_shared<map_impl_t<T, R>>(upstream, apply);
	return pipe_t<T, R>(impl, __func__);
}



// end of namespace stream
}
