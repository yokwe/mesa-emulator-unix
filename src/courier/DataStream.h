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
// DataStream.h
//

#pragma once

#include "../util/Util.h"

namespace Courier {

	class DataStream {
		QByteArray  myOwnData;
		QByteArray* myData;
		int         myBase;
		int         myPosition;
		int         myLimit;

	public:
		DataStream(QByteArray *byteArray) : myData(byteArray), myBase(0), myPosition(0), myLimit(0) {}

		DataStream() : DataStream(&myOwnData) {}
		DataStream(const DataStream& that) {
			myData       = that.myData;
			myBase       = that.myBase;
			myPosition   = that.myPosition;
			myLimit      = that.myLimit;
		}
		DataStream& operator = (const DataStream& that) {
			myData       = that.myData;
			myBase       = that.myBase;
			myPosition   = that.myPosition;
			myLimit      = that.myLimit;
			return *this;
		}

		QByteArray& data() const {
			return *myData;
		}
		// method about underline QByteBuffer
		int size() {
			return myData->size();
		}
		int capacity() {
			return myData->capacity();
		}
		void reserve(int newValue) {
			myData->reserve(newValue);
		}
		void squeeze() {
			myData->squeeze();
		}

		// cursor position
		int base() const {
			return myBase;
		}
		int position() const {
			return myPosition;
		}
		int limit() const {
			return myLimit;
		}

		// set position
		//   newValue must be between myBase and myLimit
		void position(int newValue);

		// set limit
		//   also resize underline QByteBuffer
		void limit(int newValue);

		// check remain bytes for reading
		int remaining() const {
			return myLimit - myPosition;
		}
		bool hasRemaining() const {
			return myPosition < myLimit;
		}


		// prepare for read buffer from beginning
		void rewind() {
			myPosition = myBase;
		}
		// prepare for read buffer from beginning after write
		void flip() {
			limit(myPosition);
			myPosition = myBase;
		}
		// prepare for write buffer from beginning
		void clear() {
			myPosition = myBase;
			limit(myBase);
		}

		// Create subrange ByteBuffer
		DataStream newBase(int newValue) const {
			DataStream ret(*this);
			ret.setBase(newValue);
			return ret;
		}
		DataStream newBase() const {
			return newBase(myPosition);
		}
		// set base and position too.
		void setBase(int newValue);

		// copy from data to myData + myBase
		void copyFrom(int len, const uint8_t* value);
		void copyFrom(int len, const char* value) {
			copyFrom(len, (const uint8_t*)value);
		}


		//
		// read value from DataStream
		//
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
		void read  (const int readSize, char* value) {
			read(readSize, (uint8_t*)value);
		}

		void read8 (const int index, uint8_t&  value) const;
		void read16(const int index, uint16_t& value) const;
		void read32(const int index, uint32_t& value) const;
		void read48(const int index, uint64_t& value) const;
		void read  (const int index, const int readSize, uint8_t* value) const;
		void read  (const int index, const int readSize, char*   value) const {
			read(index, readSize, (uint8_t*)value);
		}


		//
		// write value to DataStream
		//
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
		void write  (const int writeSize, const char*   value) {
			write(writeSize, (const uint8_t*)value);
		}

		void write8 (const int index, uint8_t  value);
		void write16(const int index, uint16_t value);
		void write32(const int index, uint32_t value);
		void write48(const int index, uint64_t value);
		void write  (const int index, const int writeSize, const uint8_t* value);
		void write  (const int index, const int writeSize, const char* value) {
			write(index, writeSize, (const uint8_t*)value);
		}


		//
		// operator << for primitive type
		//
		DataStream& operator << (const uint8_t& value) {
			write8(value);
			return *this;
		}
		DataStream& operator << (const uint16_t& value) {
			write16(value);
			return *this;
		}
		DataStream& operator << (const uint32_t& value) {
			write32(value);
			return *this;
		}
		DataStream& operator << (const uint64_t& value) {
			write48(value);
			return *this;
		}
		DataStream& operator << (const DataStream& value);


		//
		// operator << for primitive type
		//
		DataStream& operator >> (uint8_t& value) {
			read8(value);
			return *this;
		}
		DataStream& operator >> (uint16_t& value) {
			read16(value);
			return *this;
		}
		DataStream& operator >> (uint32_t& value) {
			read32(value);
			return *this;
		}
		DataStream& operator >> (uint64_t& value) {
			read48(value);
			return *this;
		}
		DataStream& operator >> (DataStream& value);

	};
}
