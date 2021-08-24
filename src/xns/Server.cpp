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

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-server");

#include "../util/ByteBuffer.h"
#include "../util/Network.h"
#include "../util/BPF.h"

#include "Config.h"
#include "XNS.h"

#include "Server.h"


//
// Server.cpp
//


//
// XNS::Server::Context
//
XNS::Server::Context::Context(const QString& path) {
	Config config = XNS::loadConfig(path);

	device.name    = config.interface;
	device.address = 0;
	localNet       = config.localNet;

	QList<Network::Device> list = Network::getDeviceList();
	for(auto e: list) {
		if (e.name == device.name) {
			device.address   = e.address;
		}
	}
	if (device.address == 0) {
		logger.error("Unexpected");
		logger.error("  name = %s", device.name);
		for(auto e: list) {
			logger.error("  available interface = %s", e.name);
		}
		ERROR()
	}

	driver = Network::getDriver(device);

	logger.info("device   = %20s  %s", IDP::Host::toHexaDecimalString(device.address, ":"), device.name);
	logger.info("device   = %20s  %s", IDP::Host::toDecimalString(device.address), device.name);
	logger.info("localNet = %d", localNet);
}


//
// XNS::Server::Server
//
void XNS::Server::Server::init(const QString& path) {
	processThreadPool->setObjectName("ProcessThereadPool");
	processThreadPool->setMaxThreadCount(1);

	context = Context(path);
	processThread = new ProcessThread(context, handlerMap);
	processThread->setAutoDelete(false);
}
void XNS::Server::Server::add(Handler handler) {
	quint16 socket = handler.socket();
	const char* name = handler.name();
	logger.info("add handler %-4s  %s", XNS::IDP::Socket::toString(socket), name);
	handlerMap[socket] = handler;
}
bool XNS::Server::Server::running() {
	return processThread->running();
}
void XNS::Server::Server::start() {
	if (processThread->running()) {
		logger.error("Unexpected");
		ERROR()
	} else {
		// start handler
		for(quint16 socket: handlerMap.keys()) {
			Handler handler = handlerMap[socket];
			logger.info("handler START %-4s  %s", IDP::Socket::toString(socket), handler.name());
			handler.start();
		}
		// start processThread
		logger.info("processThread START");
		processThreadPool->start(processThread);
	}
}
void XNS::Server::Server::stop() {
	if (processThread->running()) {
		logger.info("processThread STOP");
		processThread->stop();
		this->processThreadPool->waitForDone();
		// stop handler
		for(quint16 socket: handlerMap.keys()) {
			Handler handler = handlerMap[socket];
			logger.info("handler STOP  %-4s  %s", IDP::Socket::toString(socket), handler.name());
			handler.stop();
		}
	} else {
		logger.warn("processThread already stop");
	}
}


//
// XNS::Server::ProcessThread
//
bool XNS::Server::ProcessThread::running() {
	return threadRunning;
}
void XNS::Server::ProcessThread::run() {
	threadRunning = true;
	stopThread    = false;

	{
		context.driver->discard();

		int ret;
		int opErrno;
		Network::Packet packet;

		for(;;) {
			// loop until data arrive
			for(;;) {
				if (stopThread) goto exitLoop;
				ret = context.driver->select(1, opErrno);
				if (ret < 0) {
					LOG_ERRNO(opErrno);
					ERROR();
				}
				if (0 < ret) break;
			}
			if (stopThread) goto exitLoop;

			// receive one data
			{
				ret = context.driver->receive(packet.data(), packet.limit(), opErrno);
				if (ret < 0) {
					logger.warn("Unexpected");
					LOG_ERRNO(opErrno);
					continue;
				}
				packet.position(0);
				packet.limit(ret);
			}

			Ethernet ethernet;
			FROM_BYTE_BUFFER(packet, ethernet);
			Buffer level1 = ethernet.block.toBuffer();

			if (ethernet.type != Ethernet::Type::XNS) continue;

			IDP idp;
			FROM_BYTE_BUFFER(level1, idp);

			logger.info("%s", ethernet.toString());
			logger.info("    %s", idp.toString());

			quint16 socket = (quint16)idp.dstSocket;
			if (handlerMap.contains(socket)) {
				Data data(context, packet, ethernet, idp);
				handlerMap[socket].handle(data);
			} else {
				logger.warn("no handler for socket %s", IDP::Socket::toString(socket));
			}
		}
exitLoop:
		/* empty statement for label */ ;
	}

	threadRunning = false;
}
void XNS::Server::ProcessThread::stop() {
	if (running()) {
		stopThread = true;
	} else {
		logger.warn("processThread already stop");
	}
}


//
// XNS::Server::Handlers::Default
//
void XNS::Server::Handlers::Default::transmit(Data& data, IDP& idp) {
	// FIXME
	(void)data;
	(void)idp;
}
void XNS::Server::Handlers::Default::transmit(Data& data, Error& error) {
	// FIXME
	(void)data;
	(void)error;
}



//
// XNS::Server::Handlers::RIPHandler
//
void XNS::Server::Handlers::RIPHandler::handle(Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::RIP) {
		RIP rip;
		FROM_BYTE_BUFFER(level2, rip);
		receive(data, rip);
	} else if (data.idp.type == IDP::Type::ERROR_) {
		Error error;
		FROM_BYTE_BUFFER(level2, error);
		receive(data, error);
	} else {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}
void XNS::Server::Handlers::RIPHandler::transmit(Data& data, RIP& rip) {
	// FIXME
	(void)data;
	(void)rip;
}


//
// XNS::Server::Handlers::EchoHandler
//
void XNS::Server::Handlers::EchoHandler::handle(Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::ECHO) {
		Echo echo;
		FROM_BYTE_BUFFER(level2, echo);
		receive(data, echo);
	} else if (data.idp.type == IDP::Type::ERROR_) {
		Error error;
		FROM_BYTE_BUFFER(level2, error);
		receive(data, error);
	} else {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}
void XNS::Server::Handlers::EchoHandler::transmit(Data& data, Echo& echo) {
	// FIXME
	(void)data;
	(void)echo;
}


//
// XNS::Server::Handlers::CHSHandler
//
void XNS::Server::Handlers::CHSHandler::handle(Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::PEX) {
		PEX pex;
		FROM_BYTE_BUFFER(level2, pex);

		if (pex.type == PEX::Type::CHS) {
			Buffer level3 = pex.block.toBuffer();
			ExpeditedCourier exp;
			FROM_BYTE_BUFFER(level3, exp);

			receive(data, pex, exp);
		} else {
			logger.error("Unexpected");
			logger.error("    %s", data.idp.toString());
			logger.error("        PEX %s", pex.toString());
			logger.error("            %s", pex.block.toString());
			ERROR();
		}
	} else if (data.idp.type == IDP::Type::ERROR_) {
		Error error;
		FROM_BYTE_BUFFER(level2, error);
		receive(data, error);
	} else {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}
void XNS::Server::Handlers::CHSHandler::transmit(Data& data, PEX& pex, ExpeditedCourier& exp) {
	// FIXME
	(void)data;
	(void)pex;
	(void)exp;
}


//
// XNS::Server::Handlers::TimeHandler
//
void XNS::Server::Handlers::TimeHandler::handle(Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::PEX) {
		PEX pex;
		FROM_BYTE_BUFFER(level2, pex);

		if (pex.type == PEX::Type::TIME) {
			Buffer level3 = pex.block.toBuffer();
			Time time;
			FROM_BYTE_BUFFER(level3, time);

			receive(data, pex, time);
		} else {
			logger.error("Unexpected");
			logger.error("    %s", data.idp.toString());
			logger.error("        PEX %s", pex.toString());
			logger.error("            %s", pex.block.toString());
			ERROR();
		}
	} else if (data.idp.type == IDP::Type::ERROR_) {
		Error error;
		FROM_BYTE_BUFFER(level2, error);
		receive(data, error);
	} else {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}
void XNS::Server::Handlers::TimeHandler::transmit(Data& data, PEX& pex, Time& time) {
	// FIXME
	(void)data;
	(void)pex;
	(void)time;
}



