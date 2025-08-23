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
// XNS.h
//

#pragma once


#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/NameMap.h"

#include "../courier/Type.h"

#include "Config.h"

namespace XNS {
	using Network::Driver;
	using Network::Packet;
	using Courier::UINT48;
	using Courier::UINT32;
	using Courier::UINT16;
	using Courier::UINT8;
	using Courier::BLOCK;
	using Courier::Base;


	class Host : public UINT48 {
	public:
		static constexpr int     SIZE      = 6;
		static constexpr uint64_t ALL       = 0xFFFF'FFFF'FFFFULL;
		static constexpr uint64_t UNKNOWN   = 0;
		static constexpr uint64_t BFN_GVWIN = 0x0000'aa00'0e60ULL; // Boot File Number of GVWin

		// define operator =
		uint64_t operator =(const uint64_t& newValue) const {
			value(newValue);
			return newValue;
		}

		static std::string toOctalString(uint64_t value);
		static std::string toDecimalString(uint64_t value);
		static std::string toHexaDecimalString(uint64_t value, std::string sep = "");
		static uint64_t fromString(std::string string);

		std::string toOctalString() const {
			return toOctalString(value());
		}
		std::string toDecimalString() const {
			return toDecimalString(value());
		}
		std::string toHexaDecimalString(std::string sep = "") const {
			return toHexaDecimalString(value(), sep);
		}

		std::string toString() const {
			return toString(value());
		}
		static std::string toString(uint64_t value) {
			return nameMap.toString(value);
		}
		static void addNameMap(uint64_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint64_t> nameMap;
	};


	class Ethernet : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value : uint16_t {
				XNS = 0x600, IP = 0x800,
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

		static const int MINIMUM_PACKET_LENGTH = 60;
		static const int HEADER_LENGTH         = 14;

		Host  dst;
		Host  src;
		Type  type;
		BLOCK block;

		void updateBlock(const BLOCK& that) {
			block.updateBufferData(that);
		}

		// Courier::Base
		std::string toString() const;
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};


	class Checksum : public UINT16 {
	public:
		enum Value : uint16_t {
			NOCHECK = 0xFFFF,
		};

		bool isNoCheck() const {
			return value() == NOCHECK;
		}

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


	class Net : public UINT32 {
	public:
		enum Value : uint32_t {
			ALL     = 0xFFFFFFFF,
			UNKNOWN = 0,
		};

		// define operator =
		uint32_t operator =(const uint32_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return toString(value());
		}
		static std::string toString(uint32_t value) {
			return nameMap.toString(value);
		}
		static void addNameMap(uint32_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint32_t> nameMap;
	};


	class Socket : public UINT16 {
	public:
		// Well-known socket numbers range from 1 to 3000 decimal
		static const uint16_t MAX_WELLKNOWN_SOCKET = 3000;

		enum Value : uint16_t {
			RIP = 1, ECHO = 2, ERROR_ = 3, ENVOY = 4, COURIER = 5, CHS_OLD = 7, TIME = 8,
			BOOT = 10, DIAG = 19,
			CHS = 20, AUTH = 21, MAIL = 22, NETEXEC = 23, WSINFO = 24, BINDING = 28,
			GERM = 35,
			TELEDEBUG = 48,
		};

		// define operator =
		uint16_t operator =(const uint16_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return toString(value());
		}
		static std::string toString(uint16_t newValue) {
			return nameMap.toString(newValue);
		}
		static void addNameMap(uint16_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint16_t> nameMap;
	};


	class IDP : public Base {
		static const int OFFSET_CHECKSUM = 0;
		static const int OFFSET_LENGTH   = 2;

	public:
		class Type : public UINT8 {
		public:
			enum Value : uint8_t {
				RIP = 1, ECHO = 2, ERROR_ = 3, PEX = 4, SPP = 5, BOOT = 9,
			};

			// define operator =
			uint8_t operator =(const uint8_t& newValue) const {
				value(newValue);
				return newValue;
			}

			std::string toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(uint8_t value, std::string name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<uint8_t> nameMap;
		};

		// MPL is estimated to be 60 seconds by observing that in the worst case.
		// If each internetwork router hop produced a store-and-forward plus transmission delay of 1 second,
		// then the delay to traverse 15 internetwork routers would be 15 seconds.
		// This was multiplied by 4 to account for rare delays.
		// In the event that an internetwork has a very large number of low-speed leased lines, packets may be delayed for more than 60 seconds.
		static const int MPL_ORIGINAL = 60; // Maximum Packet Lifetime
		static const int MPL = MPL_ORIGINAL / 10; // Change to 1/10 of original MPL

		// IMPORTANT
		// Minimum ethernet packet length is 60 (14 bytes ethernet header + 46 bytes ehternet body)
		// To conform this requirement, padding is added.
		// Also number of byes in IDP must be even number by rule. If data length is odd, one byte is added to make length even.
		// To know actual data length of XNS packet, use IDP length field.
		// IDP length field is actual data length including IDP header(30 bytes)
		// So actual data length of IDP packet is length - 30.
		static const int MININUM_PACKET_LENGTH = Ethernet::MINIMUM_PACKET_LENGTH - Ethernet::HEADER_LENGTH; // 60 - 16 = 46
		static const int HEADER_LENGTH         = 30;

		// Set length field of IDP in ByteBuffer
		// bb.base() must point to head of IDP packet
		static uint16_t getLength(const ByteBuffer& bb);
		static void    setLength(ByteBuffer& bb, uint16_t newValue);

		// Set length field of IDP in ByteBuffer
		// bb.base() must point to head of IDP packet
		static uint16_t getChecksum(const ByteBuffer& bb);
		static void    setChecksum(ByteBuffer& bb, uint16_t newValue);

		// Compute checksum of IDP using length field
		// bb.base() must point to head of IDP packet
		static uint16_t computeChecksum(const ByteBuffer& bb);


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

		void updateBlock(const BLOCK& that) {
			block.updateBufferData(that);
		}

		// Courier::Base
		std::string toString() const;
		// fromByteBuffer will set limit of bb from length field
		void fromByteBuffer(ByteBuffer& bb);
		// toByteBuffer will add padding for odd and short length, update checksum field
		void toByteBuffer  (ByteBuffer& bb) const;
	};


	// Load config file
	Config loadConfig(const std::string& path);

	class Context {
	public:
		uint64_t address;

		Driver* driver;

		Context() : address(0), driver(nullptr) {}
		// set value to config.local
		Context(Config& config);
	};


	class Data {
		void copyFrom(const Data& that);
		void fixBlock();
	public:
		// creation time in milliseconds since unix time epoch, used to remove old entry
		int64_t   timeStamp;
		Config*  config;
		Context* context;

		// received data
		Packet   packet;
		Ethernet ethernet;
		IDP      idp;

		Data();
		~Data();

		Data(const Data& that);
		Data& operator = (const Data& that);

		Data(int64_t timeStamp_, Config* config_, Context* context_, Packet& packet_, Ethernet ethernet_, IDP idp_);
	};

}
