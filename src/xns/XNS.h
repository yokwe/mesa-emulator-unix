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

#ifndef XNS_H__
#define XNS_H__

#include <QtCore>

#include "../util/Network.h"

namespace XNS {
	class XNSBase {
	public:
		// this <= ByteBuffer
		virtual void fromBB(ByteBuffer& bb) = 0;

		// ByteBuffer <= this
		virtual void toBB(ByteBuffer& bb) const = 0;

		// string representation
		virtual QString toString() const = 0;

		virtual ~XNSBase() {}
	};

	class Host : public XNSBase {
	public:
		static constexpr int     SIZE      = 6;
		static constexpr quint64 BROADCAST = 0xFFFF'FFFF'FFFFULL;

		quint64 value;

		Host() : value(0) {}
		Host(const Host& that) : value(that.value) {}
		Host& operator =(const Host& that) {
			this->value = that.value;
			return *this;
		}
		bool operator ==(const Host& that) const {
			return this->value == that.value;
		}
		bool operator ==(const quint64 that) const {
			return this->value == that;
		}

		Host(quint64 value_) : value(value_) {}
		Host(quint8* p) {
			ByteBuffer bb(SIZE, p);
			bb.read48(value);
		}

		bool isBroadcast() const {
			return value == BROADCAST;
		}
		QString toString(QString sep) const;
		QString toOctalString() const;
		QString toDecimalString() const;

		// XNSBase
		void    fromBB(ByteBuffer& bb);
		void    toBB(ByteBuffer& bb) const;
		QString toString() const {
			return toString("");
		}
	};


	class PacketType : public XNSBase {
	public:
		quint16 packeType;

		// XNSBase
		void    fromBB(ByteBuffer& bb);
		void    toBB(ByteBuffer& bb) const;
		QString toString() const;
	};

	class Ethernet : public XNSBase {
	public:
		Host    dst;
		Host    src;
		quint16 type;

		// XNSBase
		void    fromBB(ByteBuffer& bb);
		void    toBB(ByteBuffer& bb) const;
		QString toString() const;
	};

	class IDP : public XNSBase {
	public:
		quint16 checksum;
		quint16 length;
		quint8  contro;
		quint8  type;

		quint32 dstNet;
		Host    dstHost;
		quint16 dstSocket;

		quint32 srcNet;
		Host    srcHost;
		quint16 srcSocket;

		// XNSBase
		void    fromBB(ByteBuffer& bb);
		void    toBB(ByteBuffer& bb) const;
		QString toString() const;
	};
}

#endif
