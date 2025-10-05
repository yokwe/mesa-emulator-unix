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
// ByteBuffer.h
//

#pragma once

#include <cstdint>
#include <string>

#include "Util.h"


class ByteBuffer {
protected:
	static constexpr int INVALID_POS = -1;

	int      myBase;
	int      myPosition;  // read/write offset
	int      myLimit;     // maximu written offset
	int      myCapacity;  // size of myData
	uint8_t* myData;
	int      myMarkPos;

public:
	ByteBuffer() : myBase(0), myPosition(0), myLimit(0), myCapacity(0), myData(0), myMarkPos(INVALID_POS) {}
	// assume data[0..capacity) has meaningful value
	// you can read from this ByteBuffer
	ByteBuffer(int capacity, uint8_t* data) :
		myBase(0), myPosition(0), myLimit(capacity), myCapacity(capacity), myData(data), myMarkPos(INVALID_POS) {}

	ByteBuffer(const ByteBuffer& that) :
		myBase(that.myBase), myPosition(that.myPosition), myLimit(that.myLimit),
		myCapacity(that.myCapacity), myData(that.myData), myMarkPos(that.myMarkPos) {
	}
	ByteBuffer& operator =(const ByteBuffer& that) {
		this->myBase     = that.myBase;
		this->myPosition = that.myPosition;
		this->myLimit    = that.myLimit;
		this->myCapacity = that.myCapacity;
		this->myData     = that.myData;
		this->myMarkPos  = that.myMarkPos;
		return *this;
	}
	
	// copy from data to ByteBuffer
	//void copyFrom(int len, const uint8_t* data);


	bool isNull() {
		return myData == nullptr;
	}

	// // Create subrange ByteBuffer
	// ByteBuffer newBase(int newValue) const {
	// 	ByteBuffer ret(*this);
	// 	ret.setBase(newValue);
	// 	return ret;
	// }
	// ByteBuffer newBase() const {
	// 	return newBase(myPosition);
	// }
	void setBase(int newValue) {
		myBase     = newValue;
		myPosition = newValue;
		myMarkPos  = INVALID_POS;
	}
	// void setBase() {
	// 	setBase(myPosition);
	// }

	// output as hexadecimal string between base and limit
	std::string toString(int offset = 0) const;
	std::string toStringFromBase() const {
		return toString(base());
	}
	std::string toStringFromPosition() const {
		return toString(position());
	}

	int base() const {
		return myBase;
	}
	int position() const {
		return myPosition;
	}
	int limit() const {
		return myLimit;
	}
	int capacity() const {
		return myCapacity;
	}
	uint8_t* data() const {
		return myData;
	}
	int markPos() const {
		return myMarkPos;
	}

	// returns how many bytes can read
	int remaining() const {
		return myLimit - myPosition;
	}
	bool hasRemaining() const {
		return myPosition < myLimit;
	}
	// returns size of written data
	int length() const {
		return myLimit - myBase;
	}
	bool empty() const {
		return length() == 0;
	}

	// prepare for read from beginning
	void rewind() {
		myPosition = myBase;
	}
	// prepare for read from beginning after write
	void flip() {
		myLimit    = myPosition;
		myPosition = myBase;
	}
	// prepare for write from beginning
	void clear() {
		myPosition = myBase;
		myLimit    = myBase;
	}

	// // set data
	// void data_(uint8_t* data) {
	// 	myData = data;
	// }
	// set limit
	void limit(int newValue);

	// set position
	// void position(int newValue);
	// mark current position for reset
	void mark();
	// set position to marked position
	void reset();


	// read from ByteBuffer
	void read8 (uint8_t&  value) {
		read8(myPosition, value);
		myPosition += 1;
	}
	void read16(uint16_t& value) {
		read16(myPosition, value);
		myPosition += 2;
	}
	void read32(uint32_t& value) {
		read32(myPosition, value);
		myPosition += 4;
	}
	void read48(uint64_t& value) {
		read48(myPosition, value);
		myPosition += 6;
	}

	void read  (const int readSize, uint8_t* value) {
		read(myPosition, readSize, value);
		myPosition += readSize;
	}

	void read8 (const int index, uint8_t&  value) const;
	void read16(const int index, uint16_t& value) const;
	void read32(const int index, uint32_t& value) const;
	void read48(const int index, uint64_t& value) const;
	void read  (const int index, const int readSize, uint8_t* value) const;

	// write to ByteBuffer
	void write8 (uint8_t  value) {
		write8(myPosition, value);
		myLimit = myPosition += 1;
	}
	void write16(uint16_t value) {
		write16(myPosition, value);
		myLimit = myPosition += 2;
	}
	void write32(uint32_t value) {
		write32(myPosition, value);
		myLimit = myPosition += 4;
	}
	void write48(uint64_t value) {
		write48(myPosition, value);
		myLimit = myPosition += 6;
	}

	void write  (const int writeSize, const uint8_t* value) {
		write(myPosition, writeSize, value);
		myLimit = myPosition += writeSize;
	}

	void write8 (const int index, uint8_t  value);
	void write16(const int index, uint16_t value);
	void write32(const int index, uint32_t value);
	void write48(const int index, uint64_t value);
	void write  (const int index, const int writeSize, const uint8_t* value);

	void writeZero(int n);
};

template<int N>
class ByteBufferArray : public ByteBuffer {
	uint8_t array[N];
protected:
	void copyFrom(const ByteBuffer& that) {
		if (N < that.capacity()) ERROR()

		myBase     = that.base();
		myPosition = that.position();
		myLimit    = that.limit();
//		myCapacity = that.capacity();
		myMarkPos  = that.markPos();
		// copy content of that.data[0..that.capacity)
		memcpy(myData, that.data(), that.capacity());
	}
public:
	ByteBufferArray() : ByteBuffer(N, array) {
		clear();
	}
	// ByteBufferArray
	ByteBufferArray(const ByteBufferArray& that) : ByteBuffer(N, array) {
		copyFrom(that);
	}
	ByteBufferArray& operator =(const ByteBufferArray& that) {
		copyFrom(that);
		return *this;
	}
	// ByteBuffer
	ByteBufferArray(const ByteBuffer& that) : ByteBuffer(N, array) {
		copyFrom(that);
	}
	ByteBufferArray& operator =(const ByteBuffer& that) {
		copyFrom(that);
		return *this;
	}
};