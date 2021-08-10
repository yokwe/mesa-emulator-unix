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
// xns.h
//

#include <QtCore>

#include "../util/ByteBuffer.h"

namespace XNS {
	class Address {
	public:
		static constexpr int     SIZE      = 6;
		static constexpr quint64 BROADCAST = 0xFFFF'FFFF'FFFFULL;

		quint64 value;

		Address() : value(0) {}
		Address(const Address& that) : value(that.value) {}
		Address& operator =(const Address& that) {
			this->value = that.value;
			return *this;
		}
		bool operator ==(const Address& that) const {
			return this->value == that.value;
		}
		bool operator ==(const quint64 that) const {
			return this->value == that;
		}

		Address(quint64 value_) : value(value_) {}
		Address(quint8* p) {
			ByteBuffer bb(SIZE, p);
			bb.read48(value);
		}

		bool isBroadcast() const {
			return value == BROADCAST;
		}
		QString toString(QString sep = "") const;
		QString toOcatlString() const;
	};

	class Device {
	public:
		QString name;
		Address address;

		Device() {}
		Device(const Device& that) : name(that.name), address(that.address) {}
		Device& operator =(const Device& that) {
			this->name    = that.name;
			this->address = that.address;
			return *this;
		}

		QString toString() const;

		QString toString(const Address& value) const;
	};

	class Packet : public ByteBuffer {
	public:
		static constexpr int SIZE = 1514;

		quint8  data[SIZE];

		Packet() : ByteBuffer(SIZE, data) {}

		Packet(const Packet& that) : ByteBuffer(that) {
			memcpy(data, that.data, SIZE);
		}
		Packet& operator =(const Packet& that) {
			// call ByteBuffer operator=
			ByteBuffer::operator=(that);
			memcpy(data, that.data, SIZE);
			return *this;
		}

		// endian conversion
		void swab() {
			::swab(data, data, SIZE);
		}

		QString toString() const;
	};

}
