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
// Network.h
//

#ifndef NETWORK_H__
#define NETWORK_H__

#include <string.h>
#include <stdlib.h>

#include <QtCore>

#include "../util/ByteBuffer.h"

namespace Network {
	class Packet : public ByteBuffer::Buffer {
	public:
		static constexpr int SIZE = 1514;

		Packet() : ByteBuffer::Buffer(SIZE, packetData) {}

		Packet(const ByteBuffer::Buffer& that) : ByteBuffer::Buffer() {
			deepCopy(that);
		}
		Packet& operator =(const ByteBuffer::Buffer& that) {
			deepCopy(that);
			return *this;
		}

		// endian conversion
		void swab() {
			::swab(packetData, packetData, SIZE);
		}

		QString toString(int limit = 65535) const;
	private:
		quint8  packetData[SIZE];

	protected:
		void deepCopy(const ByteBuffer::Buffer& that) {
			// copy values from that
			myBase     = that.base();
			myPosition = that.position();
			myLimit    = that.limit();
			// use packetData for myData
			myCapacity = SIZE;
			myData     = packetData;
			// reset myMarkPos
			myMarkPos  = INVALID_POS;
			// copy data from that to packetData
			memcpy(packetData, that.data(), that.capacity());
		}
	};

	class Device {
	public:
		QString name;
		quint64 address;

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
		QString toString() const;
	};

	class Driver {
	public:
		virtual int select  (int& opErrno, quint32 timeout = 1) = 0; // returns return value of of select().  default timeout is 1 second
		virtual int transmit(int& opErrno, Packet& packet) = 0; // returns return value of send()
		virtual int receive (int& opErrno, Packet& packet) = 0; // returns return value of of recv()

		// discard received packet
		virtual void discard() = 0;

		Driver(const Device& device_) : device(device_), fd(0) {}
		virtual ~Driver() {}

		QString getName() {
			return device.name;
		}
		quint64 getAddress() {
			return device.address;
		}

	protected:
		Device device;
		int    fd;
	};

	//
	// OS dependent part of network implementation
	//

	QList<Device> getDeviceList();
	Driver*       getInstance(const Device& device);
}

#endif
