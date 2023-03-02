//
//
//

#pragma once

#include <string>
#include <type_traits>

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
auto vector(std::initializer_list<T> init) {
	auto impl = std::make_shared<vector_impl_t<T>>(init);
	return source_t<T>(impl, __func__);
}
template <typename T>
auto vector(std::vector<T>& data) {
	auto impl = std::make_shared<vector_impl_t<T>>(data);
	return source_t<T>(impl, __func__);
}


//
// sink  sum
//
template <typename T, typename R=T>
auto sum(source_base_t<T>* upstream) {
	R m_sum = 0;
	while(upstream->has_next()) {
		m_sum += upstream->next();
	}
	return m_sum;
}


//
// sink  count
//
template <typename T, typename R=int>
auto count(source_base_t<T>* upstream) {
	R m_count = 0;
	while(upstream->has_next()) {
		upstream->next();
		m_count++;
	}
	return m_count;
}


//
// sink null
//
template <typename T>
void null(source_base_t<T>* upstream) {
	while(upstream->has_next()) {
		upstream->next();
	}
}


//
// pipe map
//
// R Function()(T)
template <typename T, typename R, typename Function>
struct map_impl_t : public pipe_base_t<T, R> {
	using upstream_t = source_base_t<T>;
	
	Function     m_function;

	map_impl_t(upstream_t* upstream_, Function function_) : pipe_base_t<T, R>(upstream_), m_function(function_) {}

	void close() override {}
	bool has_next() override {
		return this->m_upstream->has_next();
	}
	R next() override {
		return m_function(this->m_upstream->next());
	}
};
// NOTE function is reference
template <typename T, typename Function>
auto map(source_base_t<T>* upstream,  Function& function) {
	if constexpr (std::is_invocable_v<Function, T>) {
		using R = std::invoke_result_t<Function, T>;

		auto impl = std::make_shared<map_impl_t<T, R, Function>>(upstream, function);
		return pipe_t<T, R>(impl, __func__);
	} else {
		logger.error("function %s", demangle(typeid(Function).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}
// NOTE function is rvalue reference
template <typename T, typename Function>
auto map(source_base_t<T>* upstream,  Function&& function) {
	if constexpr (std::is_invocable_v<Function, T>) {
		using R = std::invoke_result_t<Function, T>;

		auto impl = std::make_shared<map_impl_t<T, R, Function>>(upstream, function);
		return pipe_t<T, R>(impl, __func__);
	} else {
		logger.error("function %s", demangle(typeid(Function).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}


//
// pipe filter
//
// bool Predicate()(T)
template <typename T, typename Predicate>
struct filter_impl_t : public pipe_base_t<T, T> {
	using upstream_t = source_base_t<T>;

	Predicate   m_predicate;
	bool        m_has_value = false;
	T           m_value;

	filter_impl_t(upstream_t* upstream, Predicate predicate) : pipe_base_t<T, T>(upstream), m_predicate(predicate) {}

	void close() override {}
	bool has_next() override {
		for(;;) {
			if (m_has_value) break;
			if (!this->m_upstream->has_next()) break;
			m_value = this->m_upstream->next();
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
// NOTE predicate is reference
template <typename T, typename Predicate>
auto filter(source_base_t<T>* upstream, Predicate& predicate) {
	if constexpr (std::is_invocable_v<Predicate, T>) {
		using R = std::invoke_result_t<Predicate, T>;
		static_assert(std::is_same_v<R, bool>);

//		logger.error("function %s", demangle(typeid(Predicate).name()));
//		logger.error("T        %s", demangle(typeid(T).name()));
//		logger.error("R        %s", demangle(typeid(T).name()));

		auto impl = std::make_shared<filter_impl_t<T, Predicate>>(upstream, predicate);
		return pipe_t<T, T>(impl, __func__);
	} else {
		logger.error("function %s", demangle(typeid(Predicate).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}
// NOTE predicate is rvalue reference
template <typename T, typename Predicate>
auto filter(source_base_t<T>* upstream, Predicate&& predicate) {
	if constexpr (std::is_invocable_v<Predicate, T>) {
		using R = std::invoke_result_t<Predicate, T>;
		static_assert(std::is_same_v<R, bool>);

//		logger.error("function %s", demangle(typeid(Predicate).name()));
//		logger.error("T        %s", demangle(typeid(T).name()));
//		logger.error("R        %s", demangle(typeid(T).name()));

		auto impl = std::make_shared<filter_impl_t<T, Predicate>>(upstream, predicate);
		return pipe_t<T, T>(impl, __func__);
	} else {
		logger.error("function %s", demangle(typeid(Predicate).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}


//
// pipe map_filter
//
// bool Predicate()(T)
template <typename T, typename Function, typename Predicate>
struct map_filter_impl_t : public pipe_base_t<T, T> {
	using upstream_t = source_base_t<T>;

	Function   m_function;
	Predicate  m_predicate;
	bool       m_has_value = false;
	T          m_value;

	map_filter_impl_t(upstream_t* upstream, Function function, Predicate predicate) : pipe_base_t<T, T>(upstream), m_function(function), m_predicate(predicate) {}

	void close() override {}
	bool has_next() override {
		for(;;) {
			if (m_has_value) break;
			if (!this->m_upstream->has_next()) break;
			m_value     = m_function(this->m_upstream->next());
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
// NOTE predicate is reference
template <typename T, typename Function, typename Predicate>
auto map_filter(source_base_t<T>* upstream, Function& function, Predicate& predicate) {
	if constexpr (std::is_invocable_v<Function, T>) {
		using R = std::invoke_result_t<Function, T>;

		if constexpr (std::is_invocable_v<Predicate, R>) {
			using R2 = std::invoke_result_t<Predicate, T>;
			static_assert(std::is_same_v<R2, bool>);

			auto impl = std::make_shared<map_filter_impl_t<T, Function, Predicate>>(upstream, function, predicate);
			return pipe_t<T, R>(impl, __func__);
		}
	}

	logger.error("function  %s", demangle(typeid(Function).name()));
	logger.error("predicate %s", demangle(typeid(Predicate).name()));
	logger.error("T         %s", demangle(typeid(T).name()));
	ERROR();
}
// NOTE predicate is rvalue reference
template <typename T, typename Function, typename Predicate>
auto map_filter(source_base_t<T>* upstream, Function&& function, Predicate&& predicate) {
	if constexpr (std::is_invocable_v<Function, T>) {
		using R = std::invoke_result_t<Function, T>;

		if constexpr (std::is_invocable_v<Predicate, R>) {
			using R2 = std::invoke_result_t<Predicate, T>;
			static_assert(std::is_same_v<R2, bool>);

			auto impl = std::make_shared<map_filter_impl_t<T, Function, Predicate>>(upstream, function, predicate);
			return pipe_t<T, R>(impl, __func__);
		}
	}

	logger.error("function  %s", demangle(typeid(Function).name()));
	logger.error("predicate %s", demangle(typeid(Predicate).name()));
	logger.error("T         %s", demangle(typeid(T).name()));
	ERROR();
}


//
// pipe peek
//
// void Consumer()(T)
template <typename T, typename Consumer>
struct peek_impl_t : public pipe_base_t<T, T> {
	using upstream_t  = source_base_t<T>;

	Consumer m_consumer;
	T        m_value;

	peek_impl_t(upstream_t* upstream, Consumer& consumer) : pipe_base_t<T, T>(upstream), m_consumer(consumer) {}
	~peek_impl_t() {}

	void close() override {}
	bool has_next() override {
		return this->m_upstream->has_next();
	}
	T next() override {
		m_value = this->m_upstream->next();
		m_consumer(m_value);
		return m_value;
	}
};
// NOTE consumer is reference
template <typename T, typename Consumer>
auto peek(source_base_t<T>* upstream, Consumer& consumer) {
	if constexpr (std::is_invocable_v<Consumer, T>) {
		using R = std::invoke_result_t<Consumer, T>;
		static_assert(std::is_same_v<R, void>);

//		logger.error("function %s", demangle(typeid(Function).name()));
//		logger.error("T        %s", demangle(typeid(T).name()));
//		logger.error("R        %s", demangle(typeid(T).name()));

		auto impl = std::make_shared<peek_impl_t<T, Consumer>>(upstream, consumer);
		return pipe_t<T, T>(impl, __func__);
	} else {
		logger.error("consumer %s", demangle(typeid(Consumer).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}
// NOTE consumer is rvalue reference
template <typename T, typename Consumer>
auto peek(source_base_t<T>* upstream, Consumer&& consumer) {
	if constexpr (std::is_invocable_v<Consumer, T>) {
		using R = std::invoke_result_t<Consumer, T>;
		static_assert(std::is_same_v<R, void>);

//		logger.error("function %s", demangle(typeid(Function).name()));
//		logger.error("T        %s", demangle(typeid(T).name()));
//		logger.error("R        %s", demangle(typeid(T).name()));

		auto impl = std::make_shared<peek_impl_t<T, Consumer>>(upstream, consumer);
		return pipe_t<T, T>(impl, __func__);
	} else {
		logger.error("consumer %s", demangle(typeid(Consumer).name()));
		logger.error("T        %s", demangle(typeid(T).name()));
		ERROR();
	}
}


//
// peek count
//
template <typename T>
struct count_consumer_t {
	struct context_t {
		std::string m_name;
		int         m_count = 0;

		context_t(const std::string& name) : m_name(name) {}
		~context_t() {
			logger.info("%s %d", m_name, m_count);
		}
	};

	std::shared_ptr<context_t> m_context;
	count_consumer_t(const std::string& name) {
		m_context = std::make_shared<context_t>(name);
	}

	void operator()(T newValue) {
		(void)newValue;
		m_context->m_count++;
	}
};
template <typename T>
auto count(source_base_t<T>* upstream, const std::string& name) {
	count_consumer_t<T> consumer(name);
	return stream::peek(upstream, consumer);
}



// end of namespace stream
}
