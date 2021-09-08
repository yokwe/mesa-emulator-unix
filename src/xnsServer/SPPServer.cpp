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
// SPPServer.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("spp-server");

#include <QtCore>

#include "../xns/SPP.h"

#include "../courier/Protocol.h"

#include "Server.h"
#include "SPPServer.h"


using Courier::BLOCK;
using Courier::ExpeditedCourier;
using XNS::Server::SPPServer;
using XNS::Server::SPPServerImpl;

void SPPServerImpl::handle(const Data& data, const SPP& spp) {
	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  SPP   %s  SPPServerImpl", TO_CSTRING(header), TO_CSTRING(spp.toString()));

	if (myState.remoteHost != data.idp.srcHost || myState.remoteSocket != data.idp.srcSocket || myState.remoteID != spp.idSrc) {
		// something goes wrong
		logger.error("Unexpected");
		logger.error("  expect  %04X  %s-%s", myState.remoteID,   TO_CSTRING(Host::toString(myState.remoteHost)), TO_CSTRING(Socket::toString(myState.remoteSocket)));
		logger.error("  actual  %04X  %s-%s", (quint16)spp.idSrc, TO_CSTRING(data.idp.srcHost.toString()),        TO_CSTRING(data.idp.srcSocket.toString()));
		ERROR();
	}

	if (spp.control.isSystem()) {
		if (spp.control.isSendAck()) {
			// Send reply packet
			{
				SPP reply;
				reply.control = SPP::Control::BIT_SYSTEM;
				reply.sst = SPP::SST::DATA;
				reply.idSrc = myState.localID;
				reply.idDst = myState.remoteID;
				reply.seq   = myState.seq;
				reply.ack   = myState.ack;
				reply.alloc = myState.alloc;

				Network::Packet level2;
				TO_BYTE_BUFFER(level2, reply);
				BLOCK block(level2);

				IDP idp;
				setIDP(data, IDP::Type::SPP, block, idp);

				transmit(data, idp);
			}
		} else {
			// FIXEME
		}
	} else {
		// 0003 0003 0000 0000 0000 0002 0002 0001 0001 0000 0000 0000
		// protocol3
		//      protocol3
		//           call
		//                transaction
		//                     program = 2
		//                               version = 2
		//                                    procedure = ListDomainServed
		//                                         Sink = Descriptor::immediate
		//                                              Credentials type = simple
		//                                                    Credentials value sequence 0
		//                                                        Verifier sequence 0
		// return is in bulk data of following type
		//		StreamOfDomainName: TYPE = CHOICE OF {
		//			nextSegment (0) => RECORD [
		//				segment: SEQUENCE OF DomainName,
		//				restOfStream: StreamOfDomainName],
		//			lastSegment (1) => SEQUENCE OF DomainName};
		// Clearinghouse2 use Auth1
		// Clearinghouse3 use Auth3


		// FIXME
		if (spp.control.isEndOfMessage()) {
			ByteBuffer level3 = spp.block.toBuffer();
			ExpeditedCourier exp;
			FROM_BYTE_BUFFER(level3, exp);

			QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
			QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
			logger.info("%s  SPP   %s  %s", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(exp.body.toString()));

			Packet result;
			bool useBulk;
			myServer->getServices()->call(exp.body, result, useBulk);

			if (result.limit() == 0) return;
			BLOCK block(result);

			// FIXME How to send reply with SPP?
		} else {
			// FIXME
		}
	}

}


void SPPServer::handle(const Data& data, const SPP& spp) {
	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  SPP   %s  SPPServer", TO_CSTRING(header), TO_CSTRING(spp.toString()));

	if (spp.control.isSystem() && spp.control.isSendAck()) {
		// OK
		SPPServerImpl::State state;
		Listeners* listeners = myServer->getListeners();

		// build state
		{
			QString newName = QString("%1-client").arg(name());

			state.name = strdup(newName.toUtf8().constData());

			state.time = data.timeStamp;

			state.remoteHost   = data.idp.srcHost;
			state.remoteSocket = (quint16)data.idp.srcSocket;
			state.remoteID     = spp.idSrc;

			state.localSocket  = listeners->getUnusedSocket();
			state.localID      = data.timeStamp / 100;

			state.sst   = SPP::SST::DATA;

			// first reply packet is system, so sequence number is still 0
			state.seq   = 0;
			state.ack   = 0;
			state.alloc = 0;
		}

		// create listener object and add to listeners
		{
			SPPServerImpl* newImpl = impl->clone();
			newImpl->socket(state.localSocket);
			newImpl->name(state.name);
			newImpl->autoDelete(true);
			newImpl->state(state);

			// start listening object
			listeners->add(newImpl);
			// start new listener object
			newImpl->start();
		}


		// Send reply packet
		{
			SPP reply;
			reply.control = SPP::Control::BIT_SYSTEM;
			reply.sst = SPP::SST::DATA;
			reply.idSrc = state.localID;
			reply.idDst = state.remoteID;
			reply.seq   = state.seq;
			reply.ack   = state.ack;
			reply.alloc = state.alloc;

			Network::Packet level2;
			TO_BYTE_BUFFER(level2, reply);
			BLOCK block(level2);

			IDP idp;
			setIDP(data, IDP::Type::SPP, block, idp);
			// change receiving socket to match newImpl
			idp.srcSocket = state.localSocket;

			transmit(data, idp);
		}
	} else {
		logger.error("Unexpected");
		ERROR();
	}

}

