/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/


//
// OpaqueType.h
//


#pragma once



template <
	typename T,
	typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class OpaqueType {
	// IMPORTANT
	//   To use this class as member variable of class, operator= and value(newValu) need to be "const".
	//   To make above happen, value_ become mutable
	mutable T value_;
public:
	//
	// essential constructor, destructor and copy assignment operator
	//
	// default constructor
	OpaqueType() noexcept : value_(0) {};
	// destructor
	~OpaqueType() noexcept {}
	// copy constructor
	explicit OpaqueType(const OpaqueType<T>& that) noexcept : value_(that.value_) {}
	// move constructor
	explicit OpaqueType(OpaqueType<T>&& that) noexcept : value_(that.value_) {}
//	// copy assignment
//	OpaqueType<T>& operator =(const OpaqueType<T>& that) const noexcept {
//		value_ = that.value_;
//		return *this;
//	}
//	// move assignment
//	OpaqueType<T>& operator =(OpaqueType<T>&& that) const noexcept {
//		value_ = that.value_;
//		return *this;
//	}


	//
	// with T
	//

	// copy constructor with T
	OpaqueType(const T& newValue) noexcept : value_(newValue) {}
	// move constructor with T
	OpaqueType(T&& newValue) noexcept : value_(newValue) {}
	// copy assignment with T
	T operator =(const T& newValue) const noexcept {
		value_ = newValue;
		return newValue;
	}
	// move assignment with T
	T operator =(T&& newValue) const noexcept {
		value_ = newValue;
		return newValue;
	}
	// conversion functions with T
	operator T() const {
		return value_;
	}


	// prohibit all other assignment
	template <typename X>
	T operator =(const X& newValue) const = delete;


	// access to value_
	inline T value() const {
		return value_;
	}
	inline void value(T newValue) const {
		value_ = newValue;
	}
};
