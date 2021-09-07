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
// CHSListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-chs");

#include "../util/Network.h"

#include "../courier/Protocol.h"

#include "../xnsServer/Server.h"

#include "CHSListener.h"

using Network::Packet;
using XNS::Data;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::PEX;
using Courier::BLOCK;
using Courier::Services;
using Courier::Service;
using Courier::Procedure;
using Courier::ExpeditedCourier;
using Courier::Protocol3Body;
using Courier::MessageType;
using Courier::ProgramVersion;


void CHSListener::handle(const Data& data, const PEX& pex) {
	ByteBuffer level3 = pex.block.toBuffer();
	ExpeditedCourier exp;
	FROM_BYTE_BUFFER(level3, exp);

	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(exp.body.toString()));

	Packet result;
	bool useBulk;
	myServer->getServices()->call(exp.body, result, useBulk);

	if (result.limit() == 0) return;
	BLOCK block(result);

	// result is serialized Protocol3Body

	// emulate ExpeditedCourier
	Courier::ProtocolRange range;
	range.low  = Courier::ProtocolType::PROTOCOL3;
	range.high = Courier::ProtocolType::PROTOCOL3;

	Packet result2;
	TO_BYTE_BUFFER(result2, range);
	TO_BYTE_BUFFER(result2, block);
	BLOCK block2(result2);

	PEX replyPEX;
	replyPEX.id    = pex.id;
	replyPEX.type  = PEX::Type::CHS;
	replyPEX.block = block2;

	Listener::transmit(data, replyPEX);
}

