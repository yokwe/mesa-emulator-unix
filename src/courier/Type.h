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
// Type.h
//

#pragma once

#include "../util/Util.h"

#include "../util/OpaqueType.h"
#include "../util/ByteBuffer.h"


namespace Courier {
	// helper macro to invoke fromByteBuufer / toByteBuffer
	#define FROM_BYTE_BUFFER(bb, name) name.fromByteBuffer(bb)
	#define TO_BYTE_BUFFER(bb, name) name.toByteBuffer(bb)

	class Base {
	public:
		virtual ~Base() {}

		// QString <= this
		virtual QString toString() const = 0;

		// this <= ByteBuffer
		virtual void fromByteBuffer(ByteBuffer& bb) = 0;

		// ByteBuffer <= this
		virtual void toByteBuffer(ByteBuffer& bb) const = 0;
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%u", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint8 newValue;
			bb.read8(newValue);
			value(newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%u", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint16 newValue;
			bb.read16(newValue);
			value(newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%d", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint16 newValue;
			bb.read16(newValue);
			value((qint16)newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%u", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint32 newValue;
			bb.read32(newValue);
			value(newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%d", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint32 newValue;
			bb.read32(newValue);
			value((qint32)newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
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


		//
		// Base
		//
		QString toString() const {
			return QString::asprintf("%llu", value());
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint64 newValue;
			bb.read48(newValue);
			value(newValue);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			bb.write48(value());
		}
	};

	class BLOCK : public Base {
	protected:
		ByteBuffer buffer;
	public:
		BLOCK() : buffer() {}
		BLOCK(const BLOCK& that) : buffer(that.buffer) {}
		BLOCK operator =(const BLOCK& that) {
			buffer = that.buffer;
		  return *this;
		}

		// with ByteBuffer
		explicit BLOCK(const ByteBuffer& newValue) : buffer(newValue) {}

		ByteBuffer toBuffer() const {
			return buffer;
		}

		bool isNll() {
			return buffer.isNull();
		}

		//
		// Base
		//
		QString toString() const {
			return buffer.toString();
		}
		void fromByteBuffer(ByteBuffer& bb) {
			buffer = bb.newBase();
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			int size = buffer.limit() - buffer.base();
			quint8* data = buffer.data() + buffer.base();
			bb.write(size, data);
		}
	};


	class BOOLEAN : public Base, public OpaqueType<quint16> {
	public:
		BOOLEAN() : OpaqueType() {}

		// define operator =
		bool operator =(const bool& newValue) const {
			value(newValue ? 1 : 0);
			return newValue;
		}

		// cast to bool
		operator bool() {
			return value();
		}

		//
		// Base
		//
		QString toString() const {
			return value() ? "true" : "false";
		}
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	// Use UINT16 UINT32 INT16 INT32 from ByteBuffer

	class STRING : public Base {
		const int MAX_LENGTH = 65535;
		QByteArray byteArray;
	public:
		// define operator =
		QString operator =(const QString& newValue) const;

		// cast to const char*
		operator const char* () {
			return byteArray.constData();
		}


		//
		// Base
		//
		QString toString() const {
			return byteArray.constData();
		}
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	template <typename T, unsigned short int N>
	class ARRAY : public Base {
		int length = N;
	public:
		QList<T> list;

		ARRAY() {}

		ARRAY(const QList<T>& that) {
			if (length != that.length()) {
				logger.error("Unexpected");
				logger.error("  length %d", length);
				logger.error("  that   %d", that.length());
				ERROR();
			}
			list = that;
		};
		ARRAY& operator = (const QList<T>& that) {
			if (length != that.length()) {
				logger.error("Unexpected");
				logger.error("  length %d", length);
				logger.error("  that   %d", that.length());
				ERROR();
			}
			list = that;
			return *this;
		}


		//
		// Base
		//
		QString toString() const {
			QStringList myList;
			for(auto e: list) {
				myList.append(QString("{%1}").arg(e.toString()));
			}
			return QString("(%1) {%2}").arg(myList.length()).arg(myList.join(", "));
		}
		void fromByteBuffer(ByteBuffer& bb) {
			list.clear();
			for(int i = 0; i < length; i++) {
				T newValue;
				newValue.fromByteBuffer(bb);
				list.append(newValue);
			}
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			for(auto e: list) {
				e.toByteBuffer(bb);
			}
		}
	};

	template <typename T, unsigned short int N = 65535>
	class SEQUENCE : public Base {
		quint16  maxLength = N;
	public:
		QList<T> list;

		SEQUENCE() {}

		SEQUENCE(const QList<T>& that) {
			if (maxLength < that.length()) {
				logger.error("Unexpected");
				logger.error("  maxLength %d", maxLength);
				logger.error("  that      %d", that.length());
				ERROR();
			}
			list = that;
		};
		SEQUENCE& operator = (const QList<T>& that) {
			if (maxLength < that.length()) {
				logger.error("Unexpected");
				logger.error("  maxLength %d", maxLength);
				logger.error("  that      %d", that.length());
				ERROR();
			}
			list = that;
			return *this;
		}

		int size() const {
			return list.size();
		}
		void clear() {
			list.clear();
		}
		void append(T& newValue) {
			list.append(newValue);
		}
		void append(QList<T> newValue) {
			for(auto e: newValue) {
				list.append(e);
			}
		}


		//
		// Base
		//
		QString toString() const {
			QStringList myList;
			for(auto e: list) {
				myList.append(QString("{%1}").arg(e.toString()));
			}
			return QString("(%1) {%2}").arg(myList.length()).arg(myList.join(", "));
		}
		void fromByteBuffer(ByteBuffer& bb) {
			quint16 length;
			bb.read16(length);
			if (maxLength < length) {
				logger.error("Unexpected");
				logger.error("  maxLength = %u", maxLength);
				logger.error("  length    = %u", length);
				ERROR();
			}
			list.clear();
			for(quint16 i = 0; i < length; i++) {
				T newValue;
				newValue.fromByteBuffer(bb);
				list.append(newValue);
			}
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			bb.write16((quint16)list.length());
			for(auto e: list) {
				e.toByteBuffer(bb);
			}
		}
	};


#if 0
	//
	// Define operator >> for generic class T that decived from Courier::Base
	//
	template <typename T>
	ByteBuffer& operator << (ByteBuffer &bb, const T &value) {
		static_assert(std::is_base_of<Base, T>::value, "T is not derived from Courier::Base");
		value.toByteBuffer(bb);
		return bb;
	}
	template <typename T>
	ByteBuffer& operator >> (ByteBuffer &bb, T &value) {
		static_assert(std::is_base_of<Base, T>::value, "T is not derived from Courier::Base");
		value.fromByteBuffer(bb);
		return bb;
	}

	//
	// Define specialized operator << for unsigned primitive type
	//
	template <>
	inline ByteBuffer& operator << (ByteBuffer &bb, const quint8 &value) {
		bb.write8(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const quint16 &value) {
		bb.write16(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const quint32 &value) {
		bb.write32(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const quint64 &value) {
		bb.write48(value);
		return bb;
	}

	//
	// Define specialized operator << for signed primitive type
	//
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const qint8 &value) {
		bb.write8((quint8)value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const qint16 &value) {
		bb.write16((quint16)value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const qint32 &value) {
		bb.write32((quint32)value);
		return bb;
	}

	//
	// Define specialized operator << for for Buffer
	//
	template <>
	inline ByteBuffer& operator <<(ByteBuffer &bb, const ByteBuffer &value) {
		int size = value.limit() - value.base();
		quint8* data = value.data() + value.base();
		bb.write(size, data);
		return bb;
	}

	//
	// Define specialized operator >> for unsigned primitive type
	//
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, quint8 &value) {
		bb.read8(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, quint16 &value) {
		bb.read16(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, quint32 &value) {
		bb.read32(value);
		return bb;
	}
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, quint64 &value) {
		bb.read48(value);
		return bb;
	}

	//
	// Define specialized operator >> for signed primitive type
	//
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, qint8 &value) {
		quint8 newValue;
		bb.read8(newValue);
		value = (qint8)newValue;
		return bb;
	}
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, qint16 &value) {
		quint16 newValue;
		bb.read16(newValue);
		value = (qint16)newValue;
		return bb;
	}
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, qint32 &value) {
		quint32 newValue;
		bb.read32(newValue);
		value = (qint32)newValue;
		return bb;
	}

	//
	// Define specialized operator >> for Buffer
	//
	template <>
	inline ByteBuffer& operator >>(ByteBuffer &bb, ByteBuffer &value) {
		value = bb.newBase();
		return bb;
	}
#endif

}
