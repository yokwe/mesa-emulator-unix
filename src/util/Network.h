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
// Network.h
//

#pragma once

#include <string.h>
#include <stdlib.h>


#include "../util/ByteBuffer.h"

namespace Network {

	class Packet : public ByteBuffer {
		void copyFrom(const ByteBuffer& that);
	public:
		static constexpr int SIZE = 1514;

		Packet();
		~Packet();

		Packet(const Packet& that);
		Packet& operator =(const Packet& that);

		Packet(const ByteBuffer& that);
		Packet& operator =(const ByteBuffer& that);

		// endian conversion
		void swab() {
			::swab(myPacketData, myPacketData, SIZE);
		}

		std::string toString(int limit = 65535) const;

		uint8_t* packetData();

	private:
		uint8_t  myPacketData[SIZE];
	};

	class Device {
	public:
		std::string name;
		uint64_t address;

		Device() : address(0) {}
		Device(const Device& that) : name(that.name), address(that.address) {}
		Device& operator =(const Device& that) {
			this->name    = that.name;
			this->address = that.address;
			return *this;
		}

		bool isNull() {
			return address == 0;
		}
		std::string toString() const;
	};

	class Driver {
	public:
		// no error checking
		virtual int  select  (uint32_t timeout, int& opErrno) = 0;
		virtual int  transmit(uint8_t* data, uint32_t dataLen, int& opErrno) = 0;
		virtual int  receive (uint8_t* data, uint32_t dataLen, int& opErrno, int64_t* msecSinceEpoch = nullptr) = 0;
		virtual void discard() = 0;

		Driver(const Device& device_) : device(device_) {}
		virtual ~Driver() {}

		std::string getName() {
			return device.name;
		}
		uint64_t getAddress() {
			return device.address;
		}

	protected:
		Device device;
	};

	//
	// OS dependent part of network implementation
	//

	std::vector<Device> getDeviceList();
	Device        getDevice(const std::string& name);
	Driver*       getDriver(const Device& device);
}
