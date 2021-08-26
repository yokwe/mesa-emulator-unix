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
XNS::Server::Context::Context(const Config& config) {
	localNet = 0;
	for(auto e: config.network.list) {
		if (e.hop == 0) {
			localNet = e.net;
		}
	}
	if (localNet == 0) {
		logger.error("Unexpected");
		for(auto e: config.network.list) {
			logger.error("  available network %d %d %s", e.hop, e.net, e.name);
		}
		ERROR()
	}

	Network::Device device;
	QList<Network::Device> list = Network::getDeviceList();
	for(auto e: list) {
		if (e.name == config.network.interface) {
			device = e;
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

	localAddress = device.address;
	driver = Network::getDriver(device);

	logger.info("device   = %20s  %s", Host::toHexaDecimalString(device.address, ":"), device.name);
	logger.info("device   = %20s  %s", Host::toDecimalString(device.address), device.name);
	logger.info("localNet = %d", localNet);
}


//
// XNS::Server::Server
//
void XNS::Server::Server::add(Service service) {
	if (processThread) ERROR();

	quint16 socket = service.socket();
	const char* name = service.name();
	logger.info("add service %-4s  %s", XNS::IDP::Socket::toString(socket), name);
	serviceMap[socket] = service;
}
void XNS::Server::Server::init(const QString& path) {
	processThreadPool->setObjectName("ProcessThereadPool");
	processThreadPool->setMaxThreadCount(1);

	config = XNS::loadConfig(path);
	context = Context(config);
	processThread = new ProcessThread(config, context, serviceMap);
	processThread->setAutoDelete(false);

	for(auto i = serviceMap.begin(); i != serviceMap.end(); i++) {
		const quint16& socket(i.key());
		Service& service(i.value());
		logger.info("service init  %-4s  %s", IDP::Socket::toString(socket), service.name());
		service.init(&config, &context);
	}
}
bool XNS::Server::Server::running() {
	return processThread->running();
}
void XNS::Server::Server::start() {
	if (processThread->running()) {
		logger.error("Unexpected");
		ERROR()
	} else {
		// start service
		for(auto i = serviceMap.begin(); i != serviceMap.end(); i++) {
			const quint16& socket(i.key());
			Service& service(i.value());
			logger.info("service start %-4s  %s", IDP::Socket::toString(socket), service.name());
			service.start();
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
		// stop service
		for(auto i = serviceMap.begin(); i != serviceMap.end(); i++) {
			const quint16& socket(i.key());
			Service& service(i.value());
			logger.info("service stop  %-4s  %s", IDP::Socket::toString(socket), service.name());
			service.stop();
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
			// milliseconds since epoch
			quint64 timeStamp;
			{
				ret = context.driver->receive(packet.data(), packet.capacity(), opErrno, &timeStamp);
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

			// check ethernet type
			if (ethernet.type != Ethernet::Type::XNS) continue;
			// check ethernet src
			// ignroe my own transmission
			if (ethernet.src == context.localAddress) continue;
			// check ethernet dst
			// accepth broadcast or my host
			if (ethernet.dst != Host::ALL && ethernet.dst != context.localAddress) continue;


			IDP idp;
			FROM_BYTE_BUFFER(level1, idp);

			logger.info("%s", ethernet.toString());
			logger.info("    %s", idp.toString());

			// check idp checksum
			{
				Buffer start = ethernet.block.toBuffer();
				quint16 checksum = XNS::IDP::getChecksum(start);
				if (checksum != XNS::IDP::Checksum::NOCHECK) {
					quint16 newValue = XNS::IDP::computeChecksum(start);
					if (checksum != newValue) {
						// checksum error
						logger.warn("Checksum error");
						continue;
					}
				}
			}

			quint16 socket = (quint16)idp.dstSocket;
			if (serviceMap.contains(socket)) {
				Data data(timeStamp, config, context, packet, ethernet, idp);
				serviceMap[socket].handle(data);
			} else {
				logger.warn("no service for socket %s", IDP::Socket::toString(socket));
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
// XNS::Server::Services::Default
//
void XNS::Server::Services::Default::transmit(const Context& context, quint64 dst, const IDP& idp) {
	Packet packet;
	packet.write48(dst);
	packet.write48(context.localAddress);
	packet.write16(XNS::Ethernet::Type::XNS);

	// save packet as start for setChecksum() and computeChecksum()
	Buffer start = packet.newBase();
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
		IDP::setLength(start, (quint16)length);
		// update length for later use
		idp.length = (quint16)length;
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
		quint16 newValue = IDP::computeChecksum(start);
		IDP::setChecksum(start, newValue);
		// update checksum for later use
		idp.checksum_ = newValue;
	}

	// transmit packet
	{
		int opErrno;
		int ret = context.driver->transmit(packet.data(), packet.limit(), opErrno);
		if (ret < 0) {
			logger.error("Unexpected");
			logger.error("ret = %d", ret);
			LOG_ERRNO(opErrno);
			ERROR();
		}
	}
}

void XNS::Server::Services::Default::transmit(const Data& data, const IDP& idp) {
	transmit(data.context, data.ethernet.src, idp);
}

void XNS::Server::Services::Default::init(const Data& data, quint8 type, BLOCK& block, IDP& idp) {
	idp.checksum_ = data.idp.checksum_;
	idp.length    = (quint16)0;
	idp.control   = (quint8)0;
	idp.type      = type;
	idp.dstNet    = data.idp.srcNet;
	idp.dstHost   = data.idp.srcHost;
	idp.dstSocket = data.idp.srcSocket;
	idp.srcNet    = data.context.localNet;
	idp.srcHost   = data.context.localAddress;
	idp.srcSocket = data.idp.dstSocket;
	idp.block     = block;
}
void XNS::Server::Services::Default::transmit(const Data& data, const Error& error) {
	Packet level2;
	TO_BYTE_BUFFER(level2, error);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::ERROR_, block, idp);

	transmit(data, idp);
}



//
// XNS::Server::Services::RIPService
//
void XNS::Server::Services::RIPService::handle(const Data& data) {
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
void XNS::Server::Services::RIPService::transmit(const Data& data, const RIP& rip) {
	Packet level2;
	TO_BYTE_BUFFER(level2, rip);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::RIP, block, idp);

	Default::transmit(data, idp);
}


//
// XNS::Server::Services::EchoService
//
void XNS::Server::Services::EchoService::handle(const Data& data) {
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
void XNS::Server::Services::EchoService::transmit(const Data& data, const Echo& echo) {
	Packet level2;
	TO_BYTE_BUFFER(level2, echo);
	BLOCK block(level2);

	IDP idp;
	init(data, IDP::Type::ECHO, block, idp);

	Default::transmit(data, idp);
}


//
// XNS::Server::Services::CHSService
//
void XNS::Server::Services::CHSService::handle(const Data& data) {
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
void XNS::Server::Services::CHSService::transmit(const Data& data, const PEX& pex, const ExpeditedCourier& exp) {
	// FIXME
	(void)data;
	(void)pex;
	(void)exp;
	logger.warn("XNS::Server::Services::CHSService::transmit");
}


//
// XNS::Server::Services::TimeService
//
void XNS::Server::Services::TimeService::handle(const Data& data) {
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
void XNS::Server::Services::TimeService::transmit(const Data& data, const PEX& pex, const Time& time) {
	// FIXME
	(void)data;
	(void)pex;
	(void)time;
	logger.warn("XNS::Server::Services::TimeService::transmit");
}



