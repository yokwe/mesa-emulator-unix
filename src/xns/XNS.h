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
#include "../util/ByteBuffer.h"

namespace XNS {
	class Host : public ByteBufferBase {
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

		// ByteBufferBase
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};


	// PacketType: TYPE = {routing(1), echo(2), error(3), packetExchange(4), sequencedPacket(5), bootServerPacket(9)};
	class PacketType : public ByteBufferBase {
	public:
		quint16 value;

		// ByteBufferBase
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

//	WellKnownSocket: TYPE = {
//	    routing(1), echo(2), error(3), envoy(4), courier(5), clearinghouseOld(7), time(8),
//	    boot(10), diag(19),
//	    clearinghouse(20), auth(21), mail(22), netExec(23), wsInfo(24), binding(28),
//	    germ(35),
//	    teleDebug(48)
//	};

	enum class SocketType : quint16 {
		ROUTING = 1, ECHO = 2, ERROR = 3, ENVOY = 4, COURIER = 5, CHS_OLD = 7, TIME = 8,
		BOOT = 10, DIAG = 19,
		CHS = 20, AUTH = 21, MAIL = 22, NET_EXEC = 23, WSINFO = 24, BINDING = 28,
		GERM = 35,
		TELE_DEBUG = 48,
	};




	class Socket : public ByteBufferBase {
	public:
		quint16 value;

		// ByteBufferBase
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	class Ethernet : public ByteBufferBase {
	public:
		Host    dst;
		Host    src;
		quint16 type;

		// ByteBufferBase
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	class IDP : public ByteBufferBase {
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

		// ByteBufferBase
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};
}

#endif
