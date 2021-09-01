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
// SPPStream.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("spp-stream");

#include "../xns/SPP.h"
#include "Server.h"

#include "SPPStream.h"

using ByteBuffer::Buffer;
using XNS::Data;
using XNS::IDP;
using XNS::SPP;

void SPPStream::start() {
	DefaultListener::start();
}
void SPPStream::stop() {
	DefaultListener::stop();
//	for(auto i = clientList.begin(); i != clientList.end(); i++) {
//		SPPStream& p = *i;
//		if (&p == this) {
//			delete p;
//		}
//	}
}

QMap<SPPStream::Key, SPPStream::State> SPPStream::stateMap;
QList<SPPStream*> clientList;


void SPPStream::handle(const XNS::Data& data, const XNS::SPP& spp) {
	Key key((quint64)data.idp.srcHost, (quint16)data.idp.srcSocket, spp.idSrc);

	State state;
	if (stateMap.contains(key)) {
		// existing connection
		state = stateMap[key];
	} else {
		// new connection
		state.time = data.timeStamp;

		state.remoteHost   = data.idp.srcHost;
		state.remoteSocket = (quint16)data.idp.srcSocket;
		state.remoteID     = spp.idSrc;

		state.localSocket  = listeners->getUnusedSocket();
		state.localID      = data.timeStamp / 100;

		state.sst   = XNS::SPP::SST::DATA;

		state.seq   = 0;
		state.ack   = 0;
		state.alloc = 4;
		stateMap[key] = state;

		SPPStream* client = new SPPStream("SPPStream-client", state.localSocket);
		client->setAutoDelete();
		listeners->add(client);
	}

	if (spp.control.isSystem()) {
		// system packet
		if (spp.control.isSendAck()) {
			SPP reply;
			reply.control = SPP::Control::BIT_SYSTEM;
			reply.sst = SPP::SST::DATA;
			reply.idSrc = state.localID;
			reply.idDst = state.remoteID;
			reply.seq   = state.seq;
			reply.ack   = state.ack;
			reply.alloc = state.alloc;

			logger.info("localSocket = %04X  localID = %04X", state.localSocket, state.localID);


//			DefaultListener::transmit(data, reply);
			Network::Packet level2;
			TO_BYTE_BUFFER(level2, reply);
			ByteBuffer::BLOCK block(level2);

			IDP idp;
			setIDP(data, IDP::Type::SPP, block, idp);
			idp.srcSocket = state.localSocket;

			transmit(data, idp);
			return;
		}

	} else {
		// data packet
		// collect data and send data to client
	}


}
