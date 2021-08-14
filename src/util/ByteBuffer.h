/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
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

#ifndef BYTEBUFFER_H__
#define BYTEBUFFER_H__

#include <cstdint>

#include "Util.h"


namespace ByteBuffer {
	class Buffer {
	protected:
		static constexpr int INVALID_POS = -1;

		int     myBase;
		int     myPosition;
		int     myLimit;
		int     myCapacity;
		quint8 *myData;
		int     myMarkPos;

	public:
		Buffer(int capacity, quint8* data) : myBase(0), myPosition(0), myLimit(capacity), myCapacity(capacity), myData(data), myMarkPos(INVALID_POS) {}

		Buffer() : myBase(0), myPosition(0), myLimit(0), myCapacity(0), myData(nullptr), myMarkPos(INVALID_POS) {}
		Buffer(const Buffer& that) {
			this->myBase     = that.myBase;
			this->myPosition = that.myPosition;
			this->myLimit    = that.myLimit;
			this->myCapacity = that.myCapacity;
			this->myData     = that.myData;
			this->myMarkPos  = that.myMarkPos;
		}
		Buffer& operator =(const Buffer& that) {
			this->myBase     = that.myBase;
			this->myPosition = that.myPosition;
			this->myLimit    = that.myLimit;
			this->myCapacity = that.myCapacity;
			this->myData     = that.myData;
			this->myMarkPos  = that.myMarkPos;
			return *this;
		}

		// Create subrange ByteBuffer
		Buffer newBase(int newValue) {
			Buffer ret(*this);
			ret.setBase(newValue);
			return ret;
		}
		Buffer newBase() {
			return newBase(myPosition);
		}
		void setBase(int newValue) {
			myBase     = newValue;
			myPosition = newValue;
			myMarkPos  = INVALID_POS;
		}

		// copy from data to ByteBuffer
		void copyFrom(int len, const quint8* data);

		QString toString(int limit = 65536) const;

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
		quint8* data() const {
			return myData;
		}

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
			myLimit    = myPosition;
			myPosition = myBase;
		}
		// prepare for write buffer from beginning
		void clear() {
			myPosition = myBase;
			myLimit    = myBase;
		}

		// set limit
		void limit(int newValue);

		// set position
		void position(int newValue);
		// mark current position for reset
		void mark();
		// set position to marked position
		void reset();


		// read from ByteBuffer
		void read8 (quint8&  value) {
			read8(myPosition, value);
			myPosition += 1;
		}
		void read16(quint16& value) {
			read16(myPosition, value);
			myPosition += 2;
		}
		void read32(quint32& value) {
			read32(myPosition, value);
			myPosition += 4;
		}
		void read48(quint64& value) {
			read48(myPosition, value);
			myPosition += 6;
		}
		void read  (const int readSize, quint8* value) {
			read(myPosition, readSize, value);
			myPosition += readSize;
		}

		void read8 (const int index, quint8&  value) const;
		void read16(const int index, quint16& value) const;
		void read32(const int index, quint32& value) const;
		void read48(const int index, quint64& value) const;
		void read  (const int index, const int readSize, quint8* value) const;

		// write to ByteBuffer
		void write8 (quint8  value) {
			write8(myPosition, value);
			myLimit = myPosition += 1;
		}
		void write16(quint16 value) {
			write16(myPosition, value);
			myLimit = myPosition += 2;
		}
		void write32(quint32 value) {
			write32(myPosition, value);
			myLimit = myPosition += 4;
		}
		void write48(quint64 value) {
			write48(myPosition, value);
			myLimit = myPosition += 6;
		}
		void write  (const int writeSize, const quint8* value) {
			write(myPosition, writeSize, value);
			myLimit = myPosition += writeSize;
		}

		void write8 (const int index, quint8  value);
		void write16(const int index, quint16 value);
		void write32(const int index, quint32 value);
		void write48(const int index, quint64 value);
		void write  (const int index, const int writeSize, const quint8* value);
	};

	// helper macro to invoke fromByteBuufer / toByteBuffer
	#define FROM_BYTE_BUFFER(bb, name) name.fromByteBuffer(bb)
	#define TO_BYTE_BUFFER(bb, name) name.toByteBuffer(bb)

	class Base {
	public:
		// this <= ByteBuffer
		virtual void fromByteBuffer(Buffer& bb) = 0;

		// ByteBuffer <= this
		virtual void toByteBuffer(Buffer& bb) const = 0;
		virtual ~Base() {}
	};

	class UINT8 : public Base {
	protected:
		quint8 value;
	public:
		UINT8() : value(0) {}
		UINT8(const UINT8& that) : value(that.value) {}
		UINT8 operator=(const UINT8& that) {
			this->value = that.value;
			return *this;
		}

		UINT8(quint8 newValue) : value(newValue) {}
		operator quint8() const {
			return value;
		}
		UINT8 operator =(quint8 newValue) {
			value = newValue;
			return *this;
		}

		// ByteBufferBase
		void fromByteBuffer(Buffer& bb) {
			bb.read8(value);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write8(value);
		}
	};

	class UINT16 : public Base {
	protected:
		quint16 value;
	public:
		UINT16() : value(0) {}
		UINT16(const UINT16& that) : value(that.value) {}
		UINT16 operator=(const UINT16& that) {
			this->value = that.value;
			return *this;
		}

		UINT16(quint16 newValue) : value(newValue) {}
		operator quint16() const {
			return value;
		}
		UINT16 operator =(quint16 newValue) {
			value = newValue;
			return *this;
		}

		// ByteBufferBase
		void fromByteBuffer(Buffer& bb) {
			bb.read16(value);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write16(value);
		}
	};

	class UINT32 : public Base {
	protected:
		quint32 value;
	public:
		UINT32() : value(0) {}
		UINT32(const UINT32& that) : value(that.value) {}
		UINT32 operator=(const UINT32& that) {
			this->value = that.value;
			return *this;
		}

		UINT32(quint32 newValue) : value(newValue) {}
		operator quint16() const {
			return value;
		}
		UINT32 operator =(quint32 newValue) {
			value = newValue;
			return *this;
		}

		// ByteBufferBase
		void fromByteBuffer(Buffer& bb) {
			bb.read32(value);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write32(value);
		}
	};

	class UINT48 : public Base {
	protected:
		quint64 value;
	public:
		UINT48() : value(0) {}
		UINT48(const UINT48& that) : value(that.value) {}
		UINT48 operator=(const UINT48& that) {
			this->value = that.value;
			return *this;
		}

		UINT48(quint64 newValue) : value(newValue) {}
		operator quint64() const {
			return value;
		}
		UINT48 operator =(quint64 newValue) {
			value = newValue;
			return *this;
		}

		// ByteBufferBase
		void fromByteBuffer(Buffer& bb) {
			bb.read48(value);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write48(value);
		}
	};

	class BLOCK : public Base {
	protected:
		ByteBuffer::Buffer buffer;
	public:
		BLOCK() {}
		BLOCK(const BLOCK& that) : buffer(that.buffer) {}
		BLOCK operator=(const BLOCK& that) {
			buffer = that.buffer;
		  return *this;
		}

		QString toString() const {
			return buffer.toString();
		}

		ByteBuffer::Buffer toBuffer() {
			return buffer;
		}

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb) {
			buffer = bb.newBase();
		}
		void toByteBuffer  (Buffer& bb) const {
			int size = buffer.limit() - buffer.base();
			quint8* data = buffer.data() + buffer.base();
			bb.write(size, data);
		}
	};

}

#endif
