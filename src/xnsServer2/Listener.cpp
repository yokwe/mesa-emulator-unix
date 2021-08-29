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
// Listener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listener");

#include "../courier/Service.h"

#include "Listener.h"

using Courier::Services;

//
// XNS::Server2::Listeners
//
void XNS::Server2::Listeners::add(Listener listener) {
	quint16 key = listener.socket();
	if (map.contains(key)) {
		logger.error("Unexpected");
		logger.error("  listener   %5u %s", listener.socket(), listener.name());
		ERROR();
	} else {
		map[key] = listener;
	}
}
XNS::Server2::Listener XNS::Server2::Listeners::getListener(quint16 socket) {
	if (map.contains(socket)) {
		return map[socket];
	} else {
		return Listener();
	}
}

// life cycle management
void XNS::Server2::Listeners::init(Config* config, Context* context, Services* services) {
	// call init of listener in map
	for(auto i = map.begin(); i != map.end(); i++) {
		Listener& listener = i.value();
		logger.info("Listeners::init  %5d %s", listener.socket(), listener.name());
		listener.init(config, context, services);
	}
}
void XNS::Server2::Listeners::start() {
	// call start of listener in map
	for(auto i = map.begin(); i != map.end(); i++) {
		Listener& listener = i.value();
		logger.info("Listeners::start %5d %s", listener.socket(), listener.name());
		listener.start();
	}
}
void XNS::Server2::Listeners::stop() {
	// call stop of listener in map
	for(auto i = map.begin(); i != map.end(); i++) {
		Listener& listener = i.value();
		logger.info("Listeners::stop  %5d %s", listener.socket(), listener.name());
		listener.stop();
	}
}


//
// XNS::Server2::Services::Default
//

void XNS::Server2::DefaultListener::transmit(const Context& context, quint64 dst, const IDP& idp) {
	Packet packet;
	packet.write48(dst);
	packet.write48(context.localAddress);
	packet.write16(XNS::Ethernet::Type::XNS);

	// save packet as start for setChecksum() and computeChecksum()
	Buffer start = packet.newBase();
	// write idp to packet including idp.block
	TO_BYTE_BUFFER(packet, idp);
	// reflect packet.limit() to start
	start.limit(packet.limit());

	// calculate number of padding
	int padding = 0;
	{
		// actual idp data length base on start
		int length = start.limit() - start.base();
		// set length in start
		IDP::setLength(start, (quint16)length);
		// update length for later use
		idp.length = (quint16)length;
		// padding for short length packet
		if (length < IDP::MININUM_PACKET_LENGTH) {
			padding += IDP::MININUM_PACKET_LENGTH - length;
			length = IDP::MININUM_PACKET_LENGTH;
		}
		// padding for odd length packet
		if (length % 2) {
			padding += 1;
		}
	}
	if (padding) {
		// needs padding
		for(int i = 0; i < padding; i++) {
			packet.write8(0);
		}
		// reflect packet.limit() for padding
		start.limit(start.limit() + padding);
	}

	// update checksum if necessary
	if (!idp.checksum_.isNoCheck()) {
		quint16 newValue = IDP::computeChecksum(start);
		IDP::setChecksum(start, newValue);
		// update checksum for later use
		idp.checksum_ = newValue;
	}

	// transmit packet
	{
		int opErrno;
		int ret = context.driver->transmit(packet.data(), packet.limit(), opErrno);
		if (ret < 0) {
			logger.error("Unexpected");
			logger.error("ret = %d", ret);
			LOG_ERRNO(opErrno);
			ERROR();
		}
	}
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const RIP&   rip) {
	Packet level2;
	TO_BYTE_BUFFER(level2, rip);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::RIP, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const Echo&  echo) {
	Packet level2;
	TO_BYTE_BUFFER(level2, echo);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::ECHO, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const Error& error) {
	Packet level2;
	TO_BYTE_BUFFER(level2, error);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::ERROR_, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const PEX&   pex) {
	Packet level2;
	TO_BYTE_BUFFER(level2, pex);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::PEX, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const SPP&   spp) {
	Packet level2;
	TO_BYTE_BUFFER(level2, spp);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::SPP, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::transmit(const Data& data, const Boot&  boot) {
	Packet level2;
	TO_BYTE_BUFFER(level2, boot);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::BOOT, block, idp);

	transmit(data, idp);
}
void XNS::Server2::DefaultListener::init(const Data& data, quint8 type, BLOCK& block, IDP& idp) {
	idp.checksum_ = data.idp.checksum_;
	idp.length    = (quint16)0;
	idp.control   = (quint8)0;
	idp.type      = type;
	idp.dstNet    = data.idp.srcNet;
	idp.dstHost   = data.idp.srcHost;
	idp.dstSocket = data.idp.srcSocket;
	idp.srcNet    = data.context.localNet;
	idp.srcHost   = data.context.localAddress;
	idp.srcSocket = data.idp.dstSocket;
	idp.block     = block;
}


