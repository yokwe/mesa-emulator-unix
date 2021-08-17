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
// Time.h
//

#pragma once

#include "../util/ByteBuffer.h"
#include "XNS.h"

#include <variant>

namespace XNS {
	using ByteBuffer::UINT32;
	using ByteBuffer::UINT16;
	using ByteBuffer::Buffer;
	using ByteBuffer::Base;

	class Time : public Base {
	public:
		class XNSTime : public UINT32 {
		public:
			// XNSTime represents time between 1968-01-01 00:00:00 and 2104-02-06 06:28:23.
			// The value of XNSTime is seconds from 1901-01-01 00:00:00.
			// XNSTime that represents earliest time has value 2114294400.
			static const quint32 EARLIEST_TIME = 2114294400;
			// ((1968 - 1901) * 365 days + 16 leap days) * 24 hours * 60 minutes * 60 seconds = 2114294400

			// Unix Time Epoch  1970-01-01 00:00:00
			// XNS  Time Epoch  1968-01-01 00:00:00
			//   Difference between above 2 dates are 366 + 365 = 731 days.
			static const quint32 EPOCH_DIFF = (quint32)EARLIEST_TIME + (quint32)(731 * 60 * 60 * 24);

			// unix use qint64 as seconds since eopch
			static quint32 toXNSTime(qint64 unixTime) {
				return (quint32)(unixTime + EPOCH_DIFF);
			}
			static qint64 toUnixTime(quint32 xnsTime) {
				return (qint64)xnsTime - (qint64)EPOCH_DIFF;
			}

			XNSTime() {
				value(EPOCH_DIFF);
			}
			// define operator = take unix time and set value as xns time
			quint32 operator =(const quint32& unixTime) const {
				value(toXNSTime(unixTime));
				return unixTime;
			}
			qint64 operator =(const qint64& unixTime) const {
				value(toXNSTime(unixTime));
				return unixTime;
			}
			// operator quint32() get value and returns as unix time
			operator quint32() {
				return Util::toUnixTime(value());
			}

			QString toString() const;
		};

		class Version : public UINT16 {
		public:
			enum Value : quint16 {
				CURRENT = 2,
			};

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		class Type : public UINT16 {
		public:
			enum Value : quint16 {
				REQUEST = 1, RESPONSE = 2,
			};

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		class Direction : public UINT16 {
		public:
			enum Value : quint16 {
				WEST = 0, EAST = 1,
			};

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const;
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		class Tolerance : public UINT16 {
		public:
			enum Value : quint16 {
				UNKNOWN = 0, MILLI = 1,
			};

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const;
			static void addNameMap(quint16 value, QString name);
		private:
			static QMap<quint16, QString> nameMap;
			static QMap<quint16, QString> initNameMap();
		};

		struct Request {
			// nothing
		};
		struct Response : public Base {
			XNSTime   time;             // current time between 12:00:00, 1 Jan. 1968 and 6:28:23, 6 Feb. 2104 inclusive
			Direction offsetDirection;  // east or west of prime meridian
			UINT16    offsetHours;
			UINT16    offsetMinutes;
			UINT16    dstStart;         // 0 for no DST
			UINT16    dstEnd;           // 0 for no DST
			Tolerance tolerance;
			UINT32    toleranceValue;   // supposed time error in unit of tolerance

			QString toString() const;

			// ByteBuffer::Base
			void fromByteBuffer(Buffer& bb);
			void toByteBuffer  (Buffer& bb) const;
		};

		Version   version;
		Type      type;

		std::variant<struct Request, struct Response> body;

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};

}
