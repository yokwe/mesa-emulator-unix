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
// CHService.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("svc-chs");

#include "../util/ByteBuffer.h"

#include "../courier/Protocol.h"
#include "../courier/Type.h"

#include "../xnsServer/Listener.h"

#include "CHService.h"

using Network::Packet;
using XNS::Data;
using XNS::Host;
using XNS::Socket;
using XNS::Net;
using XNS::PEX;
using XNS::Server::DefaultListener;
using Courier::Base;
using Courier::BLOCK;
using Courier::Procedure;
using Courier::Protocol3Body;


//NetworkAddress: TYPE = RECORD [
//	network: UNSPECIFIED2,
//	host: UNSPECIFIED3,
//	socket: UNSPECIFIED ];
class NetworkAddress : public Base {
public:
	Net    network;
	Host   host;
	Socket socket;

	QString toString() const;

	// ByteBuffer::Base
	void fromByteBuffer(ByteBuffer& bb);
	void toByteBuffer  (ByteBuffer& bb) const;
};

QString NetworkAddress::toString() const {
	return QString("%1-%2-%3").arg(network.toString()).arg(host.toString()).arg(socket.toString());
}

void NetworkAddress::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, network);
	FROM_BYTE_BUFFER(bb, host);
	FROM_BYTE_BUFFER(bb, socket);
}
void NetworkAddress::toByteBuffer(ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, network);
	TO_BYTE_BUFFER(bb, host);
	TO_BYTE_BUFFER(bb, socket);
}


class RetrieveAddresses : public Procedure {
	static constexpr const char*   NAME      = "RetrieveAddresses";
	static constexpr quint16       PROCEDURE = 0;
public:
	RetrieveAddresses() : Procedure(NAME, PROCEDURE) {}

	void call(const Data& data, const PEX& pex, const Protocol3Body::CallBody& body) {
		logger.info("RetrieveAddresses called");

		NetworkAddress networkAddress;
		networkAddress.network = data.context->localNet;
		networkAddress.host    = data.context->localAddress;
		networkAddress.socket  = Socket::COURIER;

		Courier::SEQUENCE<NetworkAddress, 40> reply;
		reply.append(networkAddress);

		Packet level4;
		TO_BYTE_BUFFER(level4, reply);
		BLOCK block4(level4);

		Protocol3Body::ReturnBody returnBody;
		returnBody.transaction = body.transaction;
		returnBody.block = block4;

		Courier::ExpeditedCourier exp;
		exp.range.low  = Courier::ProtocolType::PROTOCOL3;
		exp.range.high = Courier::ProtocolType::PROTOCOL3;
		exp.body.type = Courier::MessageType::RETURN;
		exp.body.set(returnBody);

		Packet level3;
		TO_BYTE_BUFFER(level3, exp);
		BLOCK block3(level3);

		// set block3 to replyPEX.block
		PEX replyPEX;
		replyPEX.id    = pex.id;
		replyPEX.type  = PEX::Type::CHS;
		replyPEX.block = block3;

		logger.info("replyPEX %s %s %s", replyPEX.toString(), exp.body.toString(), reply.toString());

		DefaultListener::transmit(data, replyPEX);
	}
};
RetrieveAddresses retrieveAddress;

void CHService::init() {
	logger.info("init %s", name());
	// add procedure
	add(&retrieveAddress);
}