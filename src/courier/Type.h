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
// Courier.h
//

#pragma once

#include "../util/Util.h"

#include "../util/OpaqueType.h"
#include "../util/ByteBuffer.h"

namespace Courier {
	using ByteBuffer::Base;
	using ByteBuffer::Buffer;

	//
	// Predefined Type
	//
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

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
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

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};

	template <typename T, unsigned short int N>
	class ARRAY : public Base {
		int length = N;
		QList<T> list;
	public:
		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb) {
			list.clear();
			for(int i = 0; i < length; i++) {
				T newValue;
				newValue.fromByteBuffer(bb);
				list.append(newValue);
			}
		}
		void toByteBuffer  (Buffer& bb) const {
			for(auto e: list) {
				e.toByteBuffer(bb);
			}
		}
	};

	template <typename T, unsigned short int N>
	class SEQUENCE : public Base {
		quint16 maxLength = N;

		QList<T> list;
	public:
		void append(T& newValue) {
			list.append(newValue);
		}
		QString toString() {
			QStringList myList;
			for(auto e: list) {
				myList.append(e.toString());
			}
			return QString("{%1}").arg(myList.join(", "));
		}
		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb) {
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
		void toByteBuffer  (Buffer& bb) const {
			bb.write16((quint16)list.length());
			for(auto e: list) {
				e.toByteBuffer(bb);
			}
		}
	};

}

