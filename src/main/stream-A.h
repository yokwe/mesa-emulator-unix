//
//
//

#pragma once


namespace streamA {

// input iterator of type T
template <typename T>
class java_iterator_t {
public:
	virtual ~java_iterator_t() {}

	virtual bool has_next() = 0;
	virtual T    next()    = 0;
};

class base_t;
class base_t {
public:
	enum type_t {
		HEAD, TAIL, BETWEEN,
	};
	base_t(type_t type, const char* name, base_t* previous) : m_type(type), m_name(name), m_previous(previous), m_closed(false) {}
	virtual ~base_t() {}

	virtual void close_impl() = 0;

	void close() {
		if (!m_closed) {
			m_closed = true;
			close_impl();
		}
	}
	bool closed() {
		return m_closed;
	}

private:
	type_t         m_type;
	const char*    m_name;
	base_t* m_previous;
	bool           m_closed;
};

// output type T using java_iterator
template <typename T>
class head_t : public base_t, public java_iterator_t<T> {
public:
	producer_t(const char* name, base_t* previous) : base_t(type_t::HEAD, name, previous) {
		assert(previous == nullptr);
	}
	virtual ~head_t() {}

	// need to implement virtual method of parent classes
	//   stream_base   close_impl()
	//   java_iterator has_next_impl()
	//   java_iterator next()

	bool has_next() override {
		if (m_closed) {
			return false;
		} else {
			bool ret = has_next_impl();
			// if no more data, call close
			if (!ret) close();
			return ret;
		}
	}
	T    next()     override {
		if (m_closed) {
			// NOT EXPECTED
		} else {
			T ret = next_impl();
			// check next data
			has_next();
			return ret;
		}
	}
};

// input type T and output type R
template <typename T, typename R>
class tail_t : public base_t, public head_t<R> {
	head_t<T>* m_head;
public:
	tail_t(const char* name, base_t* previous) : base_t(type_t::TAIL, name, previous) {
		assert(previous != nullptr);
		assert(previous->type_t == type_t::HEAD || previous->type_t == type_t::BETWEEN);
		m_head = dyamic_cast<head_t<T>*>(m_previous);
		assert(m_head != nullptr);
	}
	virtual ~tail_t() {}

	// need to implement virtual method of parent classes
	//   stream_base   close_impl()

	// consumer
	virtual void accept_impl(T& newValue) = 0;
	// get result
	virtual R    result_impl()            = 0;

	R result() {
		if (m_closed) {
			return result_impl();
		} else {
			// NOT EXPECTED
		}
	}

	void accept(T& newValue) {
		if (m_closed) {
			// NOT EXPECTED
		} else {
			accept_imp(newValue);
		}
	}

	R process() {
		while(m_head->has_next()) {
			accept(m_head->next());
		}
		return result();
	}
};


// input T data and output R data using iterator of R
// 1 -> 1
// 1 -> n
// n -> 1
// n -> m
template <typename T, typename R>
class between_t : public base_t, head_t<R> {
	head_t<T>* m_producer;
public:
	between_t(const char* name, base_t* previous): base_t(type_t::BETWEEN, name, previous) {
		assert(previous != nullptr);
		assert(previous->type_t == type_t::HEAD || previous->type_t == type_t::BETWEEN);
		m_producer = dyamic_cast<head_t<T>*>(m_previous);
		assert(m_producer != nullptr);
	}
	virtual ~between_t() {}

	// need to implement virtual method of parent classes
	//   stream_base   close_impl()
	//   java_iterator has_next_impl()
	//   java_iterator next()


	// producer
	bool has_next_impl() override {
		return m_producer->has_next();
	}
	R next_impl() override {
		// FIXME get one record of R from m_producer
		return 0;
	}
	R next_impl_11() {
		T newValue= m_producer->next();
		R ret = apply(newValue);
		return ret;
	}

};


}
