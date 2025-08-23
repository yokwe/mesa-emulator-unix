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
// Listener.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Server.h"
#include "Listener.h"

using XNS::Server::Listener;
using XNS::Server::Listeners;

//
// XNS::Server::Listeners
//
void Listeners::add(uint16_t socket, Listener* listener) {
	// sanity check
	if (server == nullptr) {
		ERROR();
	}

	QMutexLocker mutexLocker(&mapMutex);
	if (map.contains(socket)) {
		logger.error("Unexpected");
		logger.error("  listener   %5u %s", listener->socket(), listener->name());
		ERROR();
	} else {
		map[socket] = listener;

		// call init
		listener->initListener(server);
	}
}
void Listeners::remove(uint16_t socket) {
	QMutexLocker mutexLocker(&mapMutex);
	if (map.contains(socket)) {
		map.remove(socket);
	} else {
		logger.error("Unexpected");
		logger.error("  socket %s", XNS::Socket::toString(socket));
		ERROR();
	}
}
Listener* Listeners::getListener(uint16_t socket) {
	QMutexLocker mutexLocker(&mapMutex);
	if (map.contains(socket)) {
		return map[socket];
	} else {
		return nullptr;
	}
}

uint16_t Listeners::getUnusedSocket() const {
	QMutexLocker mutexLocker(&mapMutex);
	uint16_t socket;
	for(;;) {
		socket = (uint16_t)QDateTime::currentMSecsSinceEpoch();
		if (socket <= Socket::MAX_WELLKNOWN_SOCKET) continue;
		if (map.contains(socket)) continue;
		break;
	}
	return socket;
}

// life cycle management
void Listeners::start() {
	QMutexLocker mutexLocker(&mapMutex);
	// call start of listener in map
	for(auto i = map.begin(); i != map.end(); i++) {
		auto listener = i.value();
		listener->startListener();
	}
	started = true;
}
void Listeners::stop() {
	QMutexLocker mutexLocker(&mapMutex);
	// call stop of listener in map
	for(auto i = map.begin(); i != map.end();) {
		auto listener = i.value();
		listener->stopListener();

		if (listener->autoDelete()) {
			logger.info("Listeners::delete %s", listener->toString());
			delete listener;
			i = map.erase(i);
		} else {
			i++;
		}
	}
	started = false;
}


//
// XNS::Server::Listener
//
const char* Listener::toString(State value) {
	switch(value) {
	case State::NEW:
		return "NEW";
	case State::INITIALIZED:
		return "INITIALIZED";
	case State::STARTED:
		return "STARTED";
	case State::STOPPED:
		return "STOPPED";
	default:
		logger.error("Unexpected");
		logger.error("  state %d", (int)value);
		ERROR();
	}
}

void Listener::initListener  (Server* server) {
	if (myState == State::NEW) {
		logger.info("Listeners::init   %s", toString());
		init(server);
		myState = State::INITIALIZED;
		logger.info("Listeners::init   %s", toString());
	} else {
		logger.error("Unexpected");
		logger.error("  listener %s", toString());
		ERROR();
	}
}
void Listener::startListener () {
	if (myState == State::INITIALIZED || myState == State::STOPPED) {
		logger.info("Listeners::start  %s", toString());
		start();
		myState = State::STARTED;
		logger.info("Listeners::start  %s", toString());
	} else {
		logger.error("Unexpected");
		logger.error("  listener %s", toString());
		ERROR();
	}
}
void Listener::stopListener  () {
	if (myState == State::STARTED) {
		logger.info("Listeners::stop   %s", toString());
		stop();
		myState = State::STOPPED;
		logger.info("Listeners::stop   %s", toString());
	} else {
		logger.error("Unexpected");
		logger.error("  listener %s", toString());
		ERROR();
	}
}
std::string Listener::toString() {
	return std::string::asprintf("%s-%s-%s", TO_CSTRING(Socket::toString(socket())), name(), toString(myState));
}
void Listener::transmit(Driver* driver, uint64_t dst, uint64_t src, const IDP& idp) {
	Packet packet;
	packet.write48(dst);
	packet.write48(src);
	packet.write16(XNS::Ethernet::Type::XNS);

	// save packet as start for setChecksum() and computeChecksum()
	ByteBuffer start = packet.newBase();
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
		IDP::setLength(start, (uint16_t)length);
		// update length for later use
		idp.length = (uint16_t)length;
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
		uint16_t newValue = IDP::computeChecksum(start);
		IDP::setChecksum(start, newValue);
		// update checksum for later use
		idp.checksum_ = newValue;
	}

	// transmit packet
	{
		int opErrno;
		int ret = driver->transmit(packet.data(), packet.limit(), opErrno);
		if (ret < 0) {
			logger.error("Unexpected");
			logger.error("ret = %d", ret);
			LOG_ERRNO(opErrno);
			ERROR();
		}
	}
}
void Listener::transmit(const Data& data, const RIP&   rip) {
	Packet level2;
	TO_BYTE_BUFFER(level2, rip);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::RIP, block, idp);

	transmit(data, idp);
}
void Listener::transmit(const Data& data, const Echo&  echo) {
	Packet level2;
	TO_BYTE_BUFFER(level2, echo);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::ECHO, block, idp);

	transmit(data, idp);
}
void Listener::transmit(const Data& data, const Error& error) {
	Packet level2;
	TO_BYTE_BUFFER(level2, error);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::ERROR_, block, idp);

	transmit(data, idp);
}
void Listener::transmit(const Data& data, const PEX&   pex) {
	Packet level2;
	TO_BYTE_BUFFER(level2, pex);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::PEX, block, idp);

	transmit(data, idp);
}
void Listener::transmit(const Data& data, const SPP&   spp) {
	Packet level2;
	TO_BYTE_BUFFER(level2, spp);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::SPP, block, idp);

	transmit(data, idp);
}
void Listener::transmit(const Data& data, const Boot&  boot) {
	Packet level2;
	TO_BYTE_BUFFER(level2, boot);
	BLOCK block(level2);

	IDP idp;
	setIDP(data, IDP::Type::BOOT, block, idp);

	transmit(data, idp);
}
void Listener::setIDP(const Data& data, uint8_t type, BLOCK& block, IDP& idp) {
	idp.checksum_ = data.idp.checksum_;
	idp.length    = (uint16_t)0;
	idp.control   = (uint8_t)0;
	idp.type      = type;
	idp.dstNet    = data.idp.srcNet;
	idp.dstHost   = data.idp.srcHost;
	idp.dstSocket = data.idp.srcSocket;
	idp.srcNet    = data.config->local.net;
	idp.srcHost   = data.config->local.host;
	idp.srcSocket = data.idp.dstSocket;
	idp.block     = block;
}


