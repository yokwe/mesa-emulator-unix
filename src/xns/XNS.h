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

#pragma once

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/NameMap.h"

#include "Config.h"

namespace XNS {
	using ByteBuffer::UINT48;
	using ByteBuffer::UINT32;
	using ByteBuffer::UINT16;
	using ByteBuffer::UINT8;
	using ByteBuffer::BLOCK;
	using ByteBuffer::Buffer;
	using ByteBuffer::Base;


	// Load config file
	Config loadConfig(QString path);


	class IDP : public Base {
	public:
		class Checksum : public UINT16 {
		public:
			enum Value : quint16 {
				NOCHECK = 0xFFFF,
			};

			bool isNoCheck() const {
				return value() == NOCHECK;
			}

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint16 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint16> nameMap;
		};

		class Type : public UINT8 {
		public:
			enum Value : quint8 {
				RIP = 1, ECHO = 2, ERROR_ = 3, PEX = 4, SPP = 5, BOOT = 9,
			};

			// define operator =
			quint8 operator =(const quint8& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint8 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint8> nameMap;
		};

		class Net : public UINT32 {
		public:
			enum Value : quint32 {
				ALL     = 0xFFFFFFFF,
				UNKNOWN = 0,
			};

			// define operator =
			quint32 operator =(const quint32& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint32 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint32> nameMap;
		};

		class Host : public UINT48 {
		public:
			static constexpr int     SIZE      = 6;
			static constexpr quint64 ALL       = 0xFFFF'FFFF'FFFFULL;
			static constexpr quint64 UNKNOWN   = 0;

			// define operator =
			quint64 operator =(const quint64& newValue) const {
				value(newValue);
				return newValue;
			}

			static QString toOctalString(quint64 value);
			static QString toDecimalString(quint64 value);
			static QString toHexaDecimalString(quint64 value, QString sep = "");
			static quint64 fromString(QString string);

			QString toOctalString() const {
				return toOctalString(value());
			}
			QString toDecimalString() const {
				return toDecimalString(value());
			}
			QString toHexaDecimalString(QString sep = "") const {
				return toHexaDecimalString(value(), sep);
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint64 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint64> nameMap;
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

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint16 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint16> nameMap;
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

		Checksum checksum_;
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

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint16 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint16> nameMap;
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

}
