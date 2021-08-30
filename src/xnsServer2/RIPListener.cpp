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
// RIPListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-rip");

#include "RIPListener.h"

using ByteBuffer::Buffer;
using ByteBuffer::BLOCK;
using Network::Packet;
using XNS::Config;
using XNS::Context;
using XNS::Data;
using XNS::Host;
using XNS::IDP;
using XNS::RIP;
using XNS::Host;
using XNS::Server2::DefaultListener;
using Courier::Services;


RIP::Entry RIPListener::find(quint32 net) {
	for(auto e: list) {
		if (e.net == net) return e;
	}
	return RIP::Entry(net, RIP::HOP_INFINITY);
}

void RIPListener::init(Config* config_, Context* context_, Services* services_) {
	DefaultListener::init(config_, context_, services_);
	logger.info("RIPListener::init");

	list.clear();
	for(auto e: config->network.list) {
		RIP::Entry entry(e.net, e.hop);
		list.append(entry);
	}
}
void RIPListener::start() {
	stopFuture = false;
	future = QtConcurrent::run([this](){this->run();});
}
void RIPListener::stop() {
	stopFuture = true;
	future.waitForFinished();
}
void RIPListener::run() {
	int count = RIP::BROADCAST_INTERVAL - 1;
	for(;;) {
		if (stopFuture) break;
		QThread::sleep(1);
		count++;
		if (count == RIP::BROADCAST_INTERVAL) {
			count = 0;
			// transmit broadcast
			RIP rip;
			rip.type = RIP::Type::RESPONSE;
			for(auto e: list) {
				RIP::Entry entry(e.net, e.hop);
				rip.entryList.append(entry);
			}

			Packet level2;
			TO_BYTE_BUFFER(level2, rip);
			BLOCK block(level2);

			IDP idp;
			idp.checksum_ = (quint16)0;
			idp.length    = (quint16)0;
			idp.control   = (quint8)0;
			idp.type      = IDP::Type::RIP;
			idp.dstNet    = context->localNet;
			idp.dstHost   = Host::ALL;
			idp.dstSocket = IDP::Socket::RIP;
			idp.srcNet    = context->localNet;
			idp.srcHost   = context->localAddress;
			idp.srcSocket = IDP::Socket::RIP;
			idp.block     = block;

			logger.info("RIPListener periodic broadcast");
			DefaultListener::transmit(*context, Host::ALL, idp);
		}
	}
}

void RIPListener::handle(const Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::RIP) {
		RIP rip;
		FROM_BYTE_BUFFER(level2, rip);

		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  RIP   %s", TO_CSTRING(header), TO_CSTRING(rip.toString()));

		if (rip.type == RIP::Type::REQUEST) {
			RIP reply;

			reply.type = RIP::Type::RESPONSE;

			bool returnAll = false;
			if (rip.entryList.size() == 1) {
				RIP::Entry entry = rip.entryList[0];
				if (entry.net == IDP::Net::ALL && entry.hop == RIP::HOP_INFINITY) {
					returnAll = true;
				}
			}

			if (returnAll) {
				for(auto e: data.config.network.list) {
					RIP::Entry entry;
					entry.net = e.net;
					entry.hop = e.hop;
					reply.entryList.append(entry);
				}
			} else {
				for(auto e: rip.entryList) {
					reply.entryList.append(find(e.net));
				}
			}
			transmit(data, reply);
		} else {
			logger.error("Unexpected");
			logger.error("  rip  %s", rip.toString());
			ERROR();
		}

	} else if (data.idp.type == IDP::Type::ERROR_) {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}

