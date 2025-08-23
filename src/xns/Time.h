/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
	using Courier::UINT32;
	using Courier::UINT16;
	using Courier::Base;

	class Time : public Base {
	public:
		class XNSTime : public UINT32 {
		public:
			// XNSTime represents time between 1968-01-01 00:00:00 and 2104-02-06 06:28:23.
			// The value of XNSTime is seconds from 1901-01-01 00:00:00.
			// XNSTime that represents earliest time has value 2114294400.
			static const uint32_t EARLIEST_TIME = 2114294400;
			// ((1968 - 1901) * 365 days + 16 leap days) * 24 hours * 60 minutes * 60 seconds = 2114294400

			// Unix Time Epoch  1970-01-01 00:00:00
			// XNS  Time Epoch  1968-01-01 00:00:00
			//   Difference between above 2 dates are 366 + 365 = 731 days.
			static const uint32_t EPOCH_DIFF = (uint32_t)EARLIEST_TIME + (uint32_t)(731 * 60 * 60 * 24);

			// unix use int64_t as seconds since epoch
			static uint32_t toXNSTime(int64_t unixTime) {
				return (uint32_t)(unixTime + EPOCH_DIFF);
			}
			static int64_t toUnixTime(uint32_t xnsTime) {
				return (int64_t)xnsTime - (int64_t)EPOCH_DIFF;
			}

			XNSTime() {
				value(EPOCH_DIFF);
			}
			// define operator = take unix time and set value as xns time
			uint32_t operator =(const uint32_t& unixTime) const {
				value(toXNSTime(unixTime));
				return unixTime;
			}
			int64_t operator =(const int64_t& unixTime) const {
				value(toXNSTime(unixTime));
				return unixTime;
			}
			// operator uint32_t() get value and returns as unix time
			operator uint32_t() {
				return (uint32_t)toUnixTime(value());
			}
			operator int64_t() {
				return toUnixTime(value());
			}

			std::string toString() const;
		};

		class Version : public UINT16 {
		public:
			enum Value : uint16_t {
				CURRENT = 2,
			};

			// define operator =
			uint16_t operator =(const uint16_t& newValue) const {
				value(newValue);
				return newValue;
			}

			bool isCurrent() const {
				return value() == CURRENT;
			}

			std::string toString() const;
		};

		class Type : public UINT16 {
		public:
			enum Value : uint16_t {
				REQUEST = 1, RESPONSE = 2,
			};

			// define operator =
			uint16_t operator =(const uint16_t& newValue) const {
				value(newValue);
				return newValue;
			}

			std::string toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(uint16_t value, std::string name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<uint16_t> nameMap;
		};

		class Direction : public UINT16 {
		public:
			enum Value : uint16_t {
				WEST = 0, EAST = 1,
			};

			// define operator =
			uint16_t operator =(const uint16_t& newValue) const {
				value(newValue);
				return newValue;
			}

			std::string toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(uint16_t value, std::string name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<uint16_t> nameMap;
		};

		class Tolerance : public UINT16 {
		public:
			enum Value : uint16_t {
				UNKNOWN = 0, MILLI = 1,
			};

			// define operator =
			uint16_t operator =(const uint16_t& newValue) const {
				value(newValue);
				return newValue;
			}

			std::string toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(uint16_t value, std::string name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<uint16_t> nameMap;
		};

		class Response : public Base {
		public:
			XNSTime   time;             // current time between 12:00:00, 1 Jan. 1968 and 6:28:23, 6 Feb. 2104 inclusive
			Direction offsetDirection;  // east or west of prime meridian
			UINT16    offsetHours;
			UINT16    offsetMinutes;
			UINT16    dstStart;         // 0 for no DST
			UINT16    dstEnd;           // 0 for no DST
			Tolerance tolerance;
			UINT32    toleranceValue;   // supposed time error in unit of tolerance

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};

		Version   version;
		Type      type;
		std::variant<std::monostate, Response> body;

		void set(const Response& newValue);
		void get(Response& newValue) const;

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

}
