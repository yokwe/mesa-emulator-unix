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
// ServicesImpl.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("servicesImpl");


#include "ServicesImpl.h"

namespace XNS::ServicesImpl {
	void RIPService::init(Config* config_, Context* context_) {
		Default::init(config_, context_);
		list.clear();
		for(auto e: config->network.list) {
			XNS::RIP::Entry entry(e.net, e.hop);
			list.append(entry);
		}
		threadPool->setMaxThreadCount(1);
		// IMPORTANT
		// Need to set autoDelete false
		setAutoDelete(false);
	}
	void RIPService::start() {
		stopThread = false;
		threadPool->start(this);
	}
	void RIPService::stop() {
		stopThread = true;
		threadPool->waitForDone();
	}
	void RIPService::run() {
		int count = RIP::BROADCAST_INTERVAL - 1;
		for(;;) {
			if (stopThread) break;
			QThread::sleep(1);
			count++;
			if (count == RIP::BROADCAST_INTERVAL) {
				count = 0;
				// transmit broadcast
				XNS::RIP rip;
				rip.type = XNS::RIP::Type::RESPONSE;
				for(auto e: list) {
					XNS::RIP::Entry entry(e.net, e.hop);
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

				logger.info("RIPService periodic broadcast");
				Default::transmit(*context, XNS::Host::ALL, idp);
			}
		}
	}

	RIP::Entry RIPService::find(quint32 net) {
		for(auto e: list) {
			if (e.net == net) return e;
		}
		return XNS::RIP::Entry(net, XNS::RIP::HOP_INFINITY);
	}

	void RIPService::receive(const Data& data, const RIP& rip) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  RIP   %s", TO_CSTRING(header), TO_CSTRING(rip.toString()));

		if (rip.type == XNS::RIP::Type::REQUEST) {
			XNS::RIP reply;

			reply.type = XNS::RIP::Type::RESPONSE;

			bool returnAll = false;
			if (rip.entryList.size() == 1) {
				XNS::RIP::Entry entry = rip.entryList[0];
				if (entry.net == XNS::IDP::Net::ALL && entry.hop == XNS::RIP::HOP_INFINITY) {
					returnAll = true;
				}
			}

			if (returnAll) {
				for(auto e: data.config.network.list) {
					XNS::RIP::Entry entry;
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
			logger.error("  ethernet  %s", data.ethernet.toString());
			logger.error("  idp       %s", data.idp.toString());
			logger.error("  rip       %s", rip.toString());
			ERROR();
		}

	}
	void RIPService::receive(const Data& data, const Error& error) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  ERROR %s", TO_CSTRING(header), TO_CSTRING(error.toString()));
		// FIXME
	}


	void CHSService::receive(const Data& data, const PEX& pex, const ExpeditedCourier& exp) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(exp.toString()));
		// FIXME
	}
	void CHSService::receive(const Data& data, const Error& error) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  ERROR %s", TO_CSTRING(header), TO_CSTRING(error.toString()));
		// FIXME
	}


	void TimeService::receive(const Data& data, const PEX& pex, const Time& time) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(time.toString()));
		// FIXME

		if (time.type == Time::Type::REQUEST) {
			Time::Response response;
			response.time            = QDateTime::currentSecsSinceEpoch();
			response.offsetDirection = data.config.time.offsetDirection;
			response.offsetHours     = data.config.time.offsetHours;
			response.offsetMinutes   = data.config.time.offsetMinutes;
			response.tolerance       = Time::Tolerance::MILLI;
			response.toleranceValue  = 10;

			Time replyTime;
			replyTime.version = Time::Version::CURRENT;
			replyTime.type    = Time::Type::RESPONSE;
			replyTime.set(response);

			Packet level3;
			TO_BYTE_BUFFER(level3, replyTime);
			BLOCK block3(level3);

			// set block3 to replyPEX.block
			PEX replyPEX;
			replyPEX.id    = pex.id;
			replyPEX.type  = PEX::Type::TIME;
			replyPEX.block = block3;

			Default::transmit(data, replyPEX);
		} else {
			logger.error("Unexpected");
			logger.error("  time %s", time.toString());
			ERROR();
		}
	}
	void TimeService::receive(const Data& data, const Error& error) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  ERROR %s", TO_CSTRING(header), TO_CSTRING(error.toString()));
		// FIXME
	}


	void EchoService::receive(const Data& data, const Echo& echo) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  ECHO  %s", TO_CSTRING(header), TO_CSTRING(echo.toString()));

		if (echo.type == XNS::Echo::Type::REQUEST) {
			Echo reply;

			reply.type = XNS::Echo::Type::REPLY;
			reply.block = echo.block;

			transmit(data, reply);
		} else {
			logger.error("Unexpected");
			logger.error("  echo %s", echo.toString());
			ERROR();
		}
	}
	void EchoService::receive(const Data& data, const Error& error) {
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  ERROR %s", TO_CSTRING(header), TO_CSTRING(error.toString()));
		// FIXME
	}

}
