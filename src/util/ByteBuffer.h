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

#pragma once

#include <cstdint>

#include "Util.h"
#include "OpaqueType.h"


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

		bool isNull() {
			return myData == nullptr;
		}

		// Create subrange ByteBuffer
		Buffer newBase(int newValue) const {
			Buffer ret(*this);
			ret.setBase(newValue);
			return ret;
		}
		Buffer newBase() const {
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

	class UINT8 : public Base , public OpaqueType<quint8> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		UINT8() : OpaqueType() {}
		~UINT8() {}

		// with UINT8
		UINT8(const UINT8& newValue) : OpaqueType(newValue) {}
		UINT8& operator =(const UINT8& newValue) {
			value(newValue.value());
			return *this;
		}
		const UINT8& operator =(const UINT8& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with quint8
		explicit UINT8(const quint8& newValue) : OpaqueType(newValue) {}
		quint8 operator =(const quint8& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() const {
			return QString::asprintf("%u", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint8 newValue;
			bb.read8(newValue);
			value(newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write8(value());
		}
	};

	class UINT16 : public Base, public OpaqueType<quint16> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		UINT16() : OpaqueType() {}
		~UINT16() {}

		// with UINT16
		UINT16(const UINT16& newValue) : OpaqueType(newValue) {}
		UINT16& operator =(const UINT16& newValue) {
			value(newValue.value());
			return *this;
		}
		const UINT16& operator =(const UINT16& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with qunt16
		explicit UINT16(const quint16& newValue) : OpaqueType(newValue) {}
		quint16 operator =(const quint16& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() const {
			return QString::asprintf("%u", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint16 newValue;
			bb.read16(newValue);
			value(newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write16(value());
		}
	};

	class INT16 : public Base, public OpaqueType<qint16> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		INT16() : OpaqueType() {}
		~INT16() {}

		// with UINT16
		INT16(const INT16& newValue) : OpaqueType(newValue) {}
		INT16& operator =(const INT16& newValue) {
			value(newValue.value());
			return *this;
		}
		const INT16& operator =(const INT16& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with qint16
		explicit INT16(const qint16& newValue) : OpaqueType(newValue) {}
		qint16 operator =(const qint16& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() const {
			return QString::asprintf("%d", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint16 newValue;
			bb.read16(newValue);
			value((qint16)newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write16(value());
		}
	};

	class UINT32 : public Base, public OpaqueType<quint32> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		UINT32() : OpaqueType() {}
		~UINT32() {}

		// with UINT32
		UINT32& operator =(const UINT32& newValue) {
			value(newValue.value());
			return *this;
		}
		const UINT32& operator =(const UINT32& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with qunt32
		explicit UINT32(const quint32& newValue) : OpaqueType(newValue) {}
		quint32 operator =(const quint32& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() {
			return QString::asprintf("%u", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint32 newValue;
			bb.read32(newValue);
			value(newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write32(value());
		}
	};

	class INT32 : public Base, public OpaqueType<qint32> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		INT32() : OpaqueType() {}
		~INT32() {}

		// with UINT32
		INT32& operator =(const INT32& newValue) {
			value(newValue.value());
			return *this;
		}
		const INT32& operator =(const INT32& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with qunt32
		explicit INT32(const qint32& newValue) : OpaqueType(newValue) {}
		qint32 operator =(const qint32& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() const {
			return QString::asprintf("%d", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint32 newValue;
			bb.read32(newValue);
			value((qint32)newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write32(value());
		}
	};

	class UINT48 : public Base, public OpaqueType<quint64> {
	public:
		//
		// essential constructor, destructor and copy assignment operator
		//
		UINT48() : OpaqueType() {}
		~UINT48() {}

		// with UINT48
		UINT48& operator =(const UINT48& newValue) {
			value(newValue.value());
			return *this;
		}
		const UINT48& operator =(const UINT48& newValue) const {
			value(newValue.value());
			return *this;
		}

		// with quint64
		explicit UINT48(const quint64& newValue) : OpaqueType(newValue) {}
		quint64 operator =(const quint64& newValue) const {
			return OpaqueType::operator =(newValue);
		}

		QString toString() const {
			return QString::asprintf("%llu", value());
		}

		//
		// Base
		//
		void fromByteBuffer(Buffer& bb) {
			quint64 newValue;
			bb.read48(newValue);
			value(newValue);
		}
		void toByteBuffer  (Buffer& bb) const {
			bb.write48(value());
		}
	};

	class BLOCK : public Base {
	protected:
		ByteBuffer::Buffer buffer;
	public:
		BLOCK() : buffer() {}
		BLOCK(const BLOCK& that) : buffer(that.buffer) {}
		BLOCK operator =(const BLOCK& that) {
			buffer = that.buffer;
		  return *this;
		}

		// with ByteBuffer
		explicit BLOCK(const ByteBuffer::Buffer& newValue) : buffer(newValue) {}

		QString toString() const {
			return buffer.toString();
		}

		ByteBuffer::Buffer toBuffer() const {
			return buffer;
		}

		bool isNll() {
			return buffer.isNull();
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


//
// Define operator >> for generic class T that decived from ByteBuffer::Base
//
template <typename T>
ByteBuffer::Buffer& operator << (ByteBuffer::Buffer &bb, const T &value) {
	static_assert(std::is_base_of<ByteBuffer::Base, T>::value, "T is not derived from ByteBuffer::Base");
	value.toByteBuffer(bb);
	return bb;
}
template <typename T>
ByteBuffer::Buffer& operator >> (ByteBuffer::Buffer &bb, T &value) {
	static_assert(std::is_base_of<ByteBuffer::Base, T>::value, "T is not derived from ByteBuffer::Base");
	value.fromByteBuffer(bb);
	return bb;
}

//
// Define specialized operator << for unsigned primitive type
//
template <>
inline ByteBuffer::Buffer& operator << (ByteBuffer::Buffer &bb, const quint8 &value) {
	bb.write8(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const quint16 &value) {
	bb.write16(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const quint32 &value) {
	bb.write32(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const quint64 &value) {
	bb.write48(value);
	return bb;
}

//
// Define specialized operator << for signed primitive type
//
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const qint8 &value) {
	bb.write8((quint8)value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const qint16 &value) {
	bb.write16((quint16)value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const qint32 &value) {
	bb.write32((quint32)value);
	return bb;
}

//
// Define specialized operator << for for Buffer
//
template <>
inline ByteBuffer::Buffer& operator <<(ByteBuffer::Buffer &bb, const ByteBuffer::Buffer &value) {
	int size = value.limit() - value.base();
	quint8* data = value.data() + value.base();
	bb.write(size, data);
	return bb;
}

//
// Define specialized operator >> for unsigned primitive type
//
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, quint8 &value) {
	bb.read8(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, quint16 &value) {
	bb.read16(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, quint32 &value) {
	bb.read32(value);
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, quint64 &value) {
	bb.read48(value);
	return bb;
}

//
// Define specialized operator >> for signed primitive type
//
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, qint8 &value) {
	quint8 newValue;
	bb.read8(newValue);
	value = (qint8)newValue;
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, qint16 &value) {
	quint16 newValue;
	bb.read16(newValue);
	value = (qint16)newValue;
	return bb;
}
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, qint32 &value) {
	quint32 newValue;
	bb.read32(newValue);
	value = (qint32)newValue;
	return bb;
}

//
// Define specialized operator >> for Buffer
//
template <>
inline ByteBuffer::Buffer& operator >>(ByteBuffer::Buffer &bb, ByteBuffer::Buffer &value) {
	value = bb.newBase();
	return bb;
}

