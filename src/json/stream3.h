//
// stream3.h
//

#pragma once

#include <string>

#include "../util/Util.h"

namespace stream {
// begin of namespace stream


template <typename T>
struct source_base_t {
	virtual ~source_base_t() {}

	virtual void close()    = 0;
	virtual bool has_next() = 0;
	virtual T    next()     = 0;
};

//
// sink_t
//
template <typename T, typename R>
class sink_t {
public:
	using upstream_t = source_base_t<T>;

	struct base_t {
		virtual ~base_t() {}

		virtual void close()      = 0;
		virtual void accept(T& t) = 0;
		virtual R    result()     = 0;
	};
private:
	struct impl_t : public base_t {
		template<typename U>
		struct wrapper_t : public base_t {
			wrapper_t(const U& u) : m_object(u) {}

			void close() override {
				m_object.close();
			}
			void accept(T& t) override {
				m_object.accept(t);
			}
			R    result() override {
				return m_object.result();
			}

			U m_object;
		};

		template <typename U>
		impl_t(const U& u) : m_base(std::make_shared<wrapper_t<U>>(u)) {
			logger.info("impl_t constructor %s", demangle(typeid(U).name()));
		}

		void close() override {
			m_base->close();
		}
		void accept(T& t) override {
			m_base->accept(t);
		}
		R result() override {
			return m_base->result();
		}

		std::shared_ptr<base_t> m_base;
	};

	impl_t       m_impl;
	std::string  m_name;
	upstream_t*  m_upstream;
	bool         m_closed = false;
public:
	sink_t(impl_t impl, const char* name, source_base_t<T>* upstream_) : m_impl(impl),  m_name(name), m_upstream(upstream_) {}
	virtual ~sink_t() {
		close();
	}

	void close() {
		if (!m_closed) {
			m_closed = true;
			m_impl.close();
		}
	}
	R result() {
		return m_impl.result();
	}

	void accept(T& newValue) {
		m_impl.accept(newValue);
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
class source_t : public source_base_t<T> {
private:
	using base_t = source_base_t<T>;
	struct impl_t : public base_t {
		template<typename U>
		struct wrapper_t : public base_t {
			wrapper_t(const U& u) : m_object(u) {}

			void close() override {
				m_object.close();
			}
			bool has_next() override {
				return m_object.has_next();
			}
			T    next() override {
				return m_object.next();
			}

			U m_object;
		};

		template <typename U>
		impl_t(const U& u) : m_base(std::make_shared<wrapper_t<U>>(u)) {
			logger.info("impl_t constructor %s", demangle(typeid(U).name()));
		}

		void close() override {
			m_base->close();
		}
		bool has_next() override {
			return m_base->has_next();
		}
		T next() override {
			return m_base->next();
		}

		std::shared_ptr<base_t> m_base;
	};

	impl_t      m_impl;
	std::string m_name;
	bool        m_closed = false;
public:
	source_t(impl_t impl, const char* name) : m_impl(impl),  m_name(name) {}
	~source_t() {
		close();
	}

	void close() override {
		if (!m_closed) {
			m_closed = true;
			m_impl.close();
		}
	}
	bool has_next() override {
		if (m_closed) {
			return false;
		} else {
			return m_impl.has_next();
		}
	}
	T next() override {
		if (has_next()) {
			return m_impl.next();
		} else {
			// if there is no next and call next(), it is error
			assert(false);
		}
	}
};


//
// pipe_t
//
template <typename T, typename R>
class pipe_t : public source_base_t<R> {
private:
	using base_t = source_base_t<R>;
	struct impl_t : public base_t {
		template<typename U>
		struct wrapper_t : public base_t {
			wrapper_t(const U& u) : m_object(u) {}

			void close() override {
				m_object.close();
			}
			bool has_next() override {
				return m_object.has_next();
			}
			R    next() override {
				return m_object.next();
			}

			U m_object;
		};

		template <typename U>
		impl_t(const U& u) : m_base(std::make_shared<wrapper_t<U>>(u)) {
			logger.info("impl_t constructor %s", demangle(typeid(U).name()));
		}

		void close() override {
			m_base->close();
		}
		bool has_next() override {
			return m_base->has_next();
		}
		T next() override {
			return m_base->next();
		}

		std::shared_ptr<base_t> m_base;
	};

	impl_t      m_impl;
	std::string m_name;
	bool        m_closed = false;
public:
	pipe_t(impl_t impl, const char* name) : m_impl(impl),  m_name(name) {}
	~pipe_t() {
		close();
	}

	void close() override {
		if (!m_closed) {
			m_closed = true;
			m_impl.close();
		}
	}
	bool has_next() override {
		if (m_closed) {
			return false;
		} else {
			return m_impl.has_next();
		}
	}
	R next() override {
		if (has_next()) {
			return m_impl.next();
		} else {
			// if there is no next and call next(), it is error
			assert(false);
		}
	}
};


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
	if constexpr (!std::is_same_v<T, A0> || !(trait::arity == 1)) {
		logger.error("ASSERTION FAILED");
		logger.error("function %s", __func__);
		logger.error("arity %d", trait::arity);
		logger.error("R     %s", demangle(typeid(R).name()));
		logger.error("T     %s", demangle(typeid(T).name()));
		logger.error("A0    %s", demangle(typeid(A0).name()));
		ERROR();
	}

	map_impl_t<T, R> impl(upstream, apply);
	return pipe_t<T, R>(impl, __func__);
}


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
	vector_impl_t<T> impl(init);
	return source_t<T>(impl, __func__);
}
template <typename T>
source_t<T> vector(std::vector<T>& data) {
	vector_impl_t<T> impl(data);
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
	sum_impl_t<T, R> impl;
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
template<typename T, typename R=T>
sink_t<T, R> count(source_base_t<T>* upstream) {
	count_impl_t<T, R> impl;
	return sink_t<T, R>(impl, __func__, upstream);
}

// end of namespace stream
}
