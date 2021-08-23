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
static const Logger logger = Logger::getLogger("main");

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/BPF.h"

#include "../xns/Config.h"
#include "../xns/XNS.h"
#include "../xns/Server.h"
#include "../xns/RIP.h"
#include "../xns/PEX.h"
#include "../xns/Courier.h"
#include "../xns/Time.h"
#include "../xns/Echo.h"
#include "../xns/Courier.h"


class RIPHandler : public XNS::Server::Handlers::RIPHandler {
public:
	const char* name() {
		return "RIPHandler";
	}
	void handle(XNS::Server::Data& data, XNS::RIP& rip) {
		(void)data;
		logger.info("##  RIP %s", rip.toString());
	}
	void handle(XNS::Server::Data& data, XNS::Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}
};


class CHSHandler : public XNS::Server::Handlers::CHSHandler {
public:
	const char* name() {
		return "CHSHandler";
	}
	void handle(XNS::Server::Data& data, XNS::PEX& pex, XNS::Courier::ExpeditedCourier& exp) {
		(void)data;
		logger.info("##  CHS %s", pex.toString());
		logger.info("        %s", exp.toString());
	}
	void handle(XNS::Server::Data& data, XNS::Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}
};


class TimeHandler : public XNS::Server::Handlers::TimeHandler {
public:
	const char* name() {
		return "TimeHandler";
	}
	void handle(XNS::Server::Data& data, XNS::PEX& pex, XNS::Time& time) {
		(void)data;
		logger.info("##  TIME %s", pex.toString());
		logger.info("         %s", time.toString());
	}
	void handle(XNS::Server::Data& data, XNS::Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}
};


class EchoHandler : public XNS::Server::Handlers::EchoHandler {
public:
	const char* name() {
		return "EchoHandler";
	}
	void handle(XNS::Server::Data& data, XNS::Echo& echo) {
		(void)data;
		logger.info("##  ECHO %s", echo.toString());
	}
	void handle(XNS::Server::Data& data, XNS::Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}
};


void testXNSServer() {
	logger.info("START testXNSServer");

	RIPHandler  ripHandler;
	CHSHandler  chsHandler;
	TimeHandler timeHandler;
	EchoHandler echoHandler;

	XNS::Server::Server server;

	logger.info("server.init");
	server.init("tmp/run/xns-config.json");

	server.add(ripHandler);
	server.add(chsHandler);
	server.add(timeHandler);
	server.add(echoHandler);

	logger.info("server.start");
	server.start();
	logger.info("QThread::sleep");
	QThread::sleep(10);
	logger.info("server.stop");
	server.stop();
	logger.info("STOP testXNSServer");
}

int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	DEBUG_TRACE();

//	testNetwork();
	testXNSServer();

	logger.info("STOP");
	return 0;
}


