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
// XNS.h
//

#ifndef XNS_H__
#define XNS_H__

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"

#include "Config.h"

namespace XNS {
	using ByteBuffer::UINT48;
	using ByteBuffer::UINT32;
	using ByteBuffer::UINT16;
	using ByteBuffer::UINT8;
	using ByteBuffer::BLOCK;
	using ByteBuffer::Buffer;
	using ByteBuffer::Base;


	Config loadConfig(QString path);


	class IDP : public Base {
	public:
		class Checksum : public UINT16 {
		public:
			enum Value : quint16 {
				NOCHECK = 0xFFFF,
			};

			Checksum() : UINT16() {}
			quint16 operator =(quint16 newValue) const {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			bool isNoCheck() const {
				return value == NOCHECK;
			}

			static void addNameMap(quint16 value, QString name);
			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		class Type : public UINT8 {
		public:
			enum Value : quint8 {
				ROUTING = 1, ECHO = 2, ERROR_ = 3, PEX = 4, SPP = 5, BOOT = 9,
			};

			Type() : UINT8() {}
			quint8 operator =(const quint8& newValue) {
				return UINT8::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			static void addNameMap(quint8 value, QString name);
			QString toString() const;
		private:
			static QMap<quint8, QString> nameMap;
			static QMap<quint8, QString> initNameMap();
		};

		class Net : public UINT32 {
		public:
			enum Value : quint32 {
				ALL     = 0xFFFFFFFF,
				UNKNOWN = 0,
			};

			Net() : UINT32() {}
			quint32 operator =(const quint32& newValue) {
				return UINT32::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			static void addNameMap(quint32 value, QString name);
			QString toString() const;
		private:
			static QMap<quint32, QString> nameMap;
			static QMap<quint32, QString> initNameMap();
		};

		class Host : public UINT48 {
		public:
			static constexpr int     SIZE      = 6;
			static constexpr quint64 ALL       = 0xFFFF'FFFF'FFFFULL;
			static constexpr quint64 UNKNOWN   = 0;

			Host() : UINT48() {}
			Host(quint64 newValue) : UINT48(newValue) {}
			Host(quint8* p) {
				Buffer bb(SIZE, p);
				bb.read48(value);
			}
			quint64 operator =(const quint64& newValue) {
				return UINT48::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			static void addNameMap(quint64 value, QString name);
			QString toString() const;

			static QString toOctalString(quint64 value);
			static QString toDecimalString(quint64 value);
			static QString toHexaDecimalString(quint64 value, QString sep = "");
			static quint64 fromString(QString string);

			QString toOctalString() const {
				return toOctalString(value);
			}
			QString toDecimalString() const {
				return toDecimalString(value);
			}
			QString toHexaDecimalString(QString sep = "") const {
				return toHexaDecimalString(value, sep);
			}

		private:
			static QMap<quint64, QString> nameMap;
			static QMap<quint64, QString> initNameMap();
		};

		class Socket : public UINT16 {
		public:
			enum Value : quint16 {
				ROUTING = 1, ECHO = 2, ERROR_ = 3, ENVOY = 4, COURIER = 5, CHS_OLD = 7, TIME = 8,
				BOOT = 10, DIAG = 19,
				CHS = 20, AUTH = 21, MAIL = 22, NETEXEC = 23, WSINFO = 24, BINDING = 28,
				GERM = 35,
				TELEDEBUG = 48,
			};

			Socket() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			static void addNameMap(quint16 value, QString name);
			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};


		// IMPORTANT
		// Minimum ethernet packet length is 60 (16 bytes ethernet header + 46 bytes ehternet body)
		// To conform this requirement, padding is added.
		// Also number of byes in IDP must be even number by rule. If data length is odd, one byte is added to make length even.
		// To know actual data length of XNS packet, use IDP length field.
		// IDP length field is actual data length including IDP header(30 bytes)
		// So actual data length of IDP packet is length - 30.
		static const int MIN_IDP_LENGTH = 46;

		static const int OFFSET_CHECKSUM = 0;
		static const int OFFSET_LENGTH   = 2;

		Checksum checksum;
		UINT16   length;
		UINT8    control;
		Type     type;

		Net      dstNet;
		Host     dstHost;
		Socket   dstSocket;

		Net      srcNet;
		Host     srcHost;
		Socket   srcSocket;

		BLOCK    block;

		QString toString() const;

		// Calculate checksum of IDP using length field
		// Suppose bb.position() point to IDP packet
		static quint16 getChecksum(const ByteBuffer::Buffer& bb);
		static void    setChecksum(ByteBuffer::Buffer& bb, quint16 newValue);
		static quint16 computeChecksum(const ByteBuffer::Buffer& bb);


		// ByteBuffer::Base
		// fromByteBuffer will set limit of bb from length field
		void fromByteBuffer(Buffer& bb);
		// toByteBuffer will add padding for odd and short length, update checksum field
		void toByteBuffer  (Buffer& bb) const;
	};


	class Ethernet : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value : quint16 {
				XNS = 0x600, IP = 0x800,
			};

			Type() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			static void addNameMap(quint16 value, QString name);
			QString toString() const;

		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		IDP::Host dst;
		IDP::Host src;
		Type      type;
		BLOCK     block;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	class Routing : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				REQUEST = 1, RESPONSE = 2,
			};

			Type() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		class Entry : public Base {
		public:
			IDP::Net net;
			UINT16   hop;

			QString toString() const;

			// ByteBuffer::Base
			void fromByteBuffer(Buffer& bb);
			void toByteBuffer  (Buffer& bb) const;
		};

		static const quint32 BROADCAST_INTERVAL = 30; // 30 seconds

		Type         type;
		QList<Entry> entryList;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	class Echo : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				REQUEST = 1, REPLY = 2,
			};

			Type() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		Type  type;
		BLOCK block;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	class Error : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				UNSPEC               = 0, // An unspecified error is detected at destination
				BAD_CHECKSUM         = 1, // The checksum is incorrect or the packet has some other serious inconsistency detected at destination
				NO_SOCKET            = 2, // The specified socket does not exist at the specified destination host
				RESOURCE_LIMIT       = 3, // The destination cannot accept the packet due to resource limitations
				LISTEN_REJECT        = 4,
				INVALID_PACKET_TYPE  = 5,
				PROTOCOL_VIOLATION   = 6,

				UNSPECIFIED_IN_ROUTE = 01000, // An unspecified error occurred before reaching destination
				INCONSISTENT         = 01001, // The checksum is incorrect, or the packet has some other serious inconsistency before reaching destination
				CANT_GET_THERE       = 01002, // The destination host cannot be reached from here.
				EXCESS_HOPS          = 01003, // The packet has passed through 15 internet routes without reaching its destination.
				TOO_BIG              = 01004, // The packet is too large to be forwarded through some intermediate network.
			                                  // The Error Parameter field contains the length of the largest packet that can be accommodated.
				CONGESTION_WARNING   = 01005,
				CONGESTION_DISCARD   = 01006,
			};

			Type() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		Type   type;
		UINT16 param;
		BLOCK  block;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	class PEX : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				UNSPEC = 0, TIME = 1, CHS = 2, TELEDEBUG = 8,
			};

			Type() : UINT16() {}
			quint16 operator =(const quint16& newValue) {
				return UINT16::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		UINT32 id;
		Type   type;
		BLOCK  block;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	class SPP : public Base {
	public:
		// SST - Sub System Type
		class SST : public UINT8 {
		public:
			enum Value {
				// From Courier/Friends/CourierProtocol.mesa
				DATA        = 0,   // Courier
				// From Courier/Private/BulkData.mesa
				BULK        = 1,   // Bulk Data
				// From NS/Public/NetworkStream.mesa
				CLOSE       = 254, // Closing connection
				CLOSE_REPLY = 255, // Reply of CLOSE (handshake)
			};

			SST() : UINT8() {}
			quint8 operator =(const quint8& newValue) {
				return UINT8::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		private:
			static QMap<quint8, QString> nameMap;
			static QMap<quint8, QString> initNameMap();
		};
		class Control : public UINT8 {
		public:
			static const quint8 BIT_SYSTEM         = 0x80; // System Packet
			static const quint8 BIT_SEND_ACK       = 0x40; // Send Acknowledgment
			static const quint8 BIT_ATTENTION      = 0x20; // Attention
			static const quint8 BIT_END_OF_MESSAGE = 0x10; // End of Message
			static const quint8 BIT_UNUSED         = 0x0F;

			bool isSystem() const {
				return value & BIT_SYSTEM;
			}
			bool isSendAck() const {
				return value & BIT_SEND_ACK;
			}
			bool isAttention() const {
				return value & BIT_ATTENTION;
			}
			bool isEndOfMessage() const {
				return value & BIT_END_OF_MESSAGE;
			}

			Control() : UINT8() {}
			quint8 operator =(const quint8& newValue) {
				return UINT8::operator =(newValue);
			}
			// prohibit other assignment explicitly
			template <typename T> T operator =(const T& newValue) const = delete;

			QString toString() const;
		};

		Control control; // Control Bit
		SST    sst;     // Sub System Type
		UINT16 idSrc;   // connection id of source
		UINT16 idDst;   // connection id of destination
		UINT16 seq;     // sequence
		UINT16 ack;     // acknowledgment
		UINT16 alloc;   // allocation
		BLOCK  block;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};

}

#endif
