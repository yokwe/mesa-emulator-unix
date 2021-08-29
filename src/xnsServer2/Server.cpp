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
// Server.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("server");

#include "Server.h"

using XNS::Server2::Server;

//
// XNS::Server2::Server
//
void Server::init(const QString& path) {
	config = XNS::loadConfig(path);
	context = Context(config);

	listeners.init(&config, &context, &services);
	services.init();
}
void Server::start() {
	stopFuture = false;
	listeners.start();
	services.start();

	future = QtConcurrent::run([this](){this->run();});
}
void Server::stop() {
	stopFuture = true;
	future.waitForFinished();
	listeners.stop();
	services.stop();
}
void Server::run() {
	context.driver->discard();

	int ret;
	int opErrno;
	Packet level0;

	for(;;) {
		// loop until data arrive
		for(;;) {
			ret = context.driver->select(1, opErrno);
			if (ret < 0) {
				LOG_ERRNO(opErrno);
				ERROR();
			}
			if (stopFuture) goto exitLoop;
			if (0 < ret) break;
		}

		// receive one data
		quint64 msecsSinceEpoch;
		{
			ret = context.driver->receive(level0.data(), level0.capacity(), opErrno, &msecsSinceEpoch);
			if (ret < 0) {
				logger.warn("Unexpected");
				LOG_ERRNO(opErrno);
				continue;
			}
			level0.position(0);
			level0.limit(ret);
		}

		Ethernet ethernet;
		FROM_BYTE_BUFFER(level0, ethernet);
		Buffer level1 = ethernet.block.toBuffer();

		// check ethernet type
		if (ethernet.type != Ethernet::Type::XNS) continue;
		// check ethernet src
		// ignore my own transmission
		if (ethernet.src == context.localAddress) continue;
		// accept broadcast or to my host
		if (ethernet.dst != Host::ALL && ethernet.dst != context.localAddress) continue;

		IDP idp;
		FROM_BYTE_BUFFER(level1, idp);

		// build header
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(msecsSinceEpoch).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(ethernet.toString()), TO_CSTRING(idp.toString()));

		// check idp checksum
		{
			Buffer start = ethernet.block.toBuffer();
			quint16 checksum = XNS::IDP::getChecksum(start);
			if (checksum != XNS::IDP::Checksum::NOCHECK) {
				quint16 newValue = XNS::IDP::computeChecksum(start);
				if (checksum != newValue) {
					// checksum error
					logger.warn("%s  BAD CHECKSUM", header);
					continue;
				}
			}
		}

		quint16 socket = (quint16)idp.dstSocket;
		Listener listener = listeners.getListener(socket);
		if (listener.isNull()) {
			logger.warn("%s  NO HANDLER", header);
		} else {
			Data data(msecsSinceEpoch, config, context, level0, ethernet, idp);
			listener.handle(data);
		}
	}
exitLoop:
	/* empty statement for label */ ;
}
