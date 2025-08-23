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
// Network.cpp
//

#include "Util.h"
static const Logger logger(__FILE__);

#include "Network.h"


//
// Network::Packet
//
void Network::Packet::copyFrom(const ByteBuffer& that) {
	// copy values from that
	myBase     = that.base();
	myPosition = that.position();
	myLimit    = that.limit();
	// use myPacketData for myData
	myCapacity = SIZE;
	myData     = myPacketData;
	// reset myMarkPos
	myMarkPos  = INVALID_POS;
	// copy data from that to myPacketData
	memcpy(myPacketData, that.data(), that.capacity());
}

Network::Packet::Packet() : ByteBuffer(SIZE, myPacketData) {
	//
}
Network::Packet::~Packet() {
	//
	if (myData != myPacketData) {
		logger.error("Unexpected");
//		ERROR();
	}
}

Network::Packet::Packet(const Packet& that) : ByteBuffer() {
	copyFrom(that);
}
Network::Packet& Network::Packet::operator =(const Packet& that) {
	copyFrom(that);
	return *this;
}

Network::Packet::Packet(const ByteBuffer& that) : ByteBuffer() {
	copyFrom(that);
}

Network::Packet& Network::Packet::operator =(const ByteBuffer& that) {
	copyFrom(that);
	return *this;
}

std::string Network::Packet::toString(int limit) const {
	ByteBuffer bb(*this);
	bb.rewind();

	std::string ret = std::string::asprintf("(%4d) ", bb.limit());

	uint64_t dst;
	uint64_t src;
	uint16_t type;

	bb.read48(dst);
	bb.read48(src);
	bb.read16(type);

	ret += std::string::asprintf("%12llX  %12llX  %04X ", dst, src, type);
	ret += toHexString(bb.remaining(), data() + bb.position()).left(limit);

	return ret;
}

uint8_t* Network::Packet::packetData() {
	return myPacketData;
}


//
// Network::Device
//
std::string Network::Device::toString() const {
	return std::string("{%1 %2}").arg(name).arg(std::string("%1").arg(address, 0, 16).toUpper());
}
