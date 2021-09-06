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
#include "../util/Network.h"

#include "../xnsServer/Listener.h"

#include "../courier/Protocol.h"
#include "../courier/Type.h"

#include "../courier/Clearinghouse2.h"

#include "CHService.h"

using Network::Packet;
using XNS::Config;
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

using Courier::Clearinghouse2::NetworkAddress;

class RetrieveAddresses : public Procedure {
	static constexpr const char*   NAME      = "RetrieveAddresses";
	static constexpr quint16       PROCEDURE = 0;
	static constexpr bool          USE_BULK  = false;
public:
	RetrieveAddresses() : Procedure(NAME, PROCEDURE, USE_BULK) {}

	void call(const Config& config, const Protocol3Body::CallBody& callBody, ByteBuffer& result) {
		logger.info("%s called", NAME);

		Courier::Clearinghouse2::RetrieveAddress::Return returnValue;

		{
			NetworkAddress networkAddress;
			// send myself
			networkAddress.net    = config.local.net;
			networkAddress.host   = config.local.host;
			networkAddress.socket = Socket::COURIER;

			returnValue.address.list.append(networkAddress);
		}

		Packet level4;
		TO_BYTE_BUFFER(level4, returnValue);
		BLOCK block4(level4);

		Protocol3Body::ReturnBody returnBody;
		returnBody.transaction = callBody.transaction;
		returnBody.block = block4;

		Courier::ExpeditedCourier replyExp;
		replyExp.range.low  = Courier::ProtocolType::PROTOCOL3;
		replyExp.range.high = Courier::ProtocolType::PROTOCOL3;
		replyExp.body.type  = Courier::MessageType::RETURN;
		replyExp.body.set(returnBody);

		TO_BYTE_BUFFER(result, replyExp);
	}
};
RetrieveAddresses retrieveAddress;


class ListDomainServed : public Procedure {
	static constexpr const char*   NAME      = "ListDomainServed";
	static constexpr quint16       PROCEDURE = 1;
	static constexpr bool          USE_BULK  = true;
public:
	ListDomainServed() : Procedure(NAME, PROCEDURE, USE_BULK) {}

	void call(const Config& config, const Protocol3Body::CallBody& callBody, ByteBuffer& result) {
		(void)config;
		(void)result;
		logger.info("%s called", NAME);

		ByteBuffer bb = callBody.block.toBuffer();
		Courier::Clearinghouse2::ListDomainServed::Call callValue;
		FROM_BYTE_BUFFER(bb, callValue);

		logger.info("callValue %s", callValue.toString());

	}
};
ListDomainServed listDomainServed;


void CHService::init() {
	logger.info("init %s", name());
	// add procedure
	add(&retrieveAddress);
	add(&listDomainServed);
}
