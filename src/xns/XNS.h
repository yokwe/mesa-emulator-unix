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
	using ByteBuffer::UINT48;
	using ByteBuffer::UINT32;
	using ByteBuffer::UINT16;
	using ByteBuffer::UINT8;
	using ByteBuffer::Buffer;
	using ByteBuffer::Base;

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

		static void addNameMap(quint64 value, QString name) {
			nameMap[value] = name;
		}
		QString toName() const {
			if (nameMap.contains(value)) {
				return nameMap[value];
			} else {
				return toString();
			}
		}
		QString toString() const {
			return toHexaDecimalString("");
		}

		QString toOctalString() const;
		QString toDecimalString() const;
		QString toHexaDecimalString(QString sep) const;
		QString toHexaDecimalString() const {
			return toHexaDecimalString("-");
		}
	private:
		static QMap<quint64, QString> nameMap;
		// initNameMap will be called in source file like this
		//   QMap<quint64, QString> nameMap = initNameMap();
		static QMap<quint64, QString> initNameMap() {
			QMap<quint64, QString> ret;
			ret[ALL]     = "ALL";
			ret[UNKNOWN] = "UNKNOWN";
			return ret;
		}
	};

	class Ethernet : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value : quint16 {
				XNS = 0x600, IP = 0x800,
			};

			Type() : UINT16() {}

			static void addNameMap(quint16 value, QString name) {
				nameMap[value] = name;
			}
			QString toName() const {
				if (nameMap.contains(value)) {
					return nameMap[value];
				} else {
					return toString();
				}
			}
			QString toString() const {
				return ("");
			}

		private:
			static QMap<quint16, QString> nameMap;
			// initNameMap will be called in source file like this
			//   QMap<quint64, QString> nameMap = initNameMap();
			static QMap<quint16, QString> initNameMap() {
				QMap<quint16, QString> ret;

				nameMap[XNS] = "XNS";
				nameMap[IP]  = "IP";
				return ret;
			}
		};
		Host dst;
		Host src;
		Type type;

		QString toString() const {
			return QString("%1-%2-%3").arg(dst.toName()).arg(src.toName()).arg(type.toName());
		}

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb) {
			dst.fromByteBuffer(bb);
			src.fromByteBuffer(bb);
			type.fromByteBuffer(bb);
		}
		void toByteBuffer  (Buffer& bb) const {
			dst.toByteBuffer(bb);
			src.toByteBuffer(bb);
			type.toByteBuffer(bb);
		}
	};

	class Net : public UINT32 {
	public:
		enum Value : quint32 {
			ALL     = 0xFFFFFFFF,
			UNKNOWN = 0,
		};

		Net() : UINT32() {}

		static void addNameMap(quint32 value, QString name) {
			nameMap[value] = name;
		}
		QString toName() const {
			if (nameMap.contains(value)) {
				return nameMap[value];
			} else {
				return toString();
			}
		}
		QString toString() const {
			return QString::asprintf("%04X", value);
		}
	private:
		static QMap<quint32, QString> nameMap;
		// initNameMap will be called in source file like this
		//   QMap<quint64, QString> nameMap = initNameMap();
		static QMap<quint32, QString> initNameMap() {
			QMap<quint32, QString> ret;
			ret[ALL]     = "ALL";
			ret[UNKNOWN] = "UNKNOWN";
			return ret;
		}
	};

	class Type : public UINT8 {
	public:
		enum Value : quint8 {
			ROUTING = 1, ECHO = 2, ERROR = 3, PEX = 4, SPP = 5, BOOT = 9,
		};

		Type() : UINT8() {}

		static void addNameMap(quint8 value, QString name) {
			nameMap[value] = name;
		}
		QString toName() const {
			if (nameMap.contains(value)) {
				return nameMap[value];
			} else {
				return toString();
			}
		}
		QString toString() const {
			return QString::asprintf("%d", value);
		}
	private:
		static QMap<quint8, QString> nameMap;
		// initNameMap will be called in source file like this
		//   QMap<quint64, QString> nameMap = initNameMap();
		static QMap<quint8, QString> initNameMap() {
			QMap<quint8, QString> ret;
			ret[ROUTING] = "ROUTING";
			ret[ECHO]    = "ECHO";
			ret[ERROR]   = "ERROR";
			ret[PEX]     = "PEX";
			ret[SPP]     = "SPP";
			ret[BOOT]    = "BOOT";
			return ret;
		}
	};

	class Socket : public UINT16 {
	public:
		enum Value : quint16 {
			ROUTING = 1, ECHO = 2, ERROR = 3, ENVOY = 4, COURIER = 5, CHS_OLD = 7, TIME = 8,
			BOOT = 10, DIAG = 19,
			CHS = 20, AUTH = 21, MAIL = 22, NETEXEC = 23, WSINFO = 24, BINDING = 28,
			GERM = 35,
			TELEDEBUG = 48,
		};

		Socket() : UINT16() {}

		static void addNameMap(quint16 value, QString name) {
			nameMap[value] = name;
		}
		QString toName() const {
			if (nameMap.contains(value)) {
				return nameMap[value];
			} else {
				return toString();
			}
		}
		QString toString() const {
			return QString::asprintf("%04X", value);
		}
	private:
		static QMap<quint16, QString> nameMap;
		// initNameMap will be called in source file like this
		//   QMap<quint64, QString> nameMap = initNameMap();
		static QMap<quint16, QString> initNameMap() {
			QMap<quint16, QString> ret;

			ret[ROUTING]  = "ROUTING";
			ret[ECHO]      = "ECHO";
			ret[ERROR]     = "ERROR";
			ret[ENVOY]     = "ENVOYE";
			ret[COURIER]   = "COURIER";
			ret[CHS_OLD]   = "CHS_OLD";
			ret[TIME]      = "TIME";
			ret[BOOT]      = "BOOT";
			ret[DIAG]      = "DIAG";

			ret[CHS]       = "CHS";
			ret[AUTH]      = "AUTH";
			ret[MAIL]      = "MAIL";
			ret[NETEXEC]   = "NETEXEC";
			ret[WSINFO]    = "WSINFO";
			ret[BINDING]   = "BINDING";

			ret[GERM]      = "GERM";
			ret[TELEDEBUG] = "TELEDEBUG";

			return ret;
		}
	};

	class IDP : public Base {
	public:
		UINT16  checksum;
		UINT16  length;
		UINT8   control;
		Type    type;

		Net     dstNet;
		Host    dstHost;
		Socket  dstSocket;

		Net     srcNet;
		Host    srcHost;
		Socket  srcSocket;

		QString toString() const {
			return QString("%1 %2 %3 %4  %5-%6-%7  %8-%9-%10").
				arg((quint16)checksum, 4, 16, QChar('0')).
				arg((quint16)length, 4).
				arg((quint8)control, 2, 16, QChar('0')).
				arg(type.toName()).
				arg(dstNet.toName()).arg(dstHost.toName()).arg(dstSocket.toName()).
				arg(srcNet.toName()).arg(srcHost.toName()).arg(srcSocket.toName());
		}

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb) {
			checksum.fromByteBuffer(bb);
			length.fromByteBuffer(bb);
			control.fromByteBuffer(bb);
			type.fromByteBuffer(bb);

			dstNet.fromByteBuffer(bb);
			dstHost.fromByteBuffer(bb);
			dstSocket.fromByteBuffer(bb);

			srcNet.fromByteBuffer(bb);
			srcHost.fromByteBuffer(bb);
			srcSocket.fromByteBuffer(bb);
		}
		void toByteBuffer  (Buffer& bb) const {
			checksum.toByteBuffer(bb);
			length.toByteBuffer(bb);
			control.toByteBuffer(bb);
			type.toByteBuffer(bb);

			dstNet.toByteBuffer(bb);
			dstHost.toByteBuffer(bb);
			dstSocket.toByteBuffer(bb);

			srcNet.toByteBuffer(bb);
			srcHost.toByteBuffer(bb);
			srcSocket.toByteBuffer(bb);
		}

	};
}

#endif
