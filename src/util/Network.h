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

#pragma once

#include <string.h>
#include <stdlib.h>

#include <QtCore>

#include "../util/ByteBuffer.h"

namespace Network {

	class Packet : public ByteBuffer {
	public:
		static constexpr int SIZE = 1514;

		Packet() : ByteBuffer(SIZE, packetData) {}

		Packet(const ByteBuffer& that) : ByteBuffer() {
			deepCopy(that);
		}
		Packet& operator =(const ByteBuffer& that) {
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
		void deepCopy(const ByteBuffer& that) {
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
		// no error checking
		virtual int  select  (quint32 timeout, int& opErrno) = 0;
		virtual int  transmit(quint8* data, quint32 dataLen, int& opErrno) = 0;
		virtual int  receive (quint8* data, quint32 dataLen, int& opErrno, qint64* msecSinceEpoch = nullptr) = 0;
		virtual void discard() = 0;

		Driver(const Device& device_) : device(device_) {}
		virtual ~Driver() {}

		QString getName() {
			return device.name;
		}
		quint64 getAddress() {
			return device.address;
		}

	protected:
		Device device;
	};

	//
	// OS dependent part of network implementation
	//

	QList<Device> getDeviceList();
	Driver*       getDriver(const Device& device);
}
