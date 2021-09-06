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
// main.c
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xnsServer");

#include "Server.h"
#include "SPPServer.h"

#include "../xnsServerImpl/TimeListener.h"
#include "../xnsServerImpl/EchoListener.h"
#include "../xnsServerImpl/RIPListener.h"
#include "../xnsServerImpl/CHSListener.h"
#include "../xnsServerImpl/SPPCourier.h"

#include "../courier/Clearinghouse2.h"

#include "../courierImpl/CHService.h"


using XNS::Server::Server;
using XNS::Server::SPPCourier;
using XNS::Server::SPPServer;

int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	DEBUG_TRACE();

	logger.info("START testXNSServer");

	EchoListener    echoListener;
	RIPListener     ripListener;
	TimeListener    timeListener;
	CHSListener     chsListener;

	SPPCourier      sppCourier;
	SPPServer       sppServerCourie(&sppCourier);

	CHService chService2("CHService2", Courier::Clearinghouse2::PROGRAM, Courier::Clearinghouse2::VERSION);
//	CHService chService3("CHService3", Courier::CHS::PROGRAM, Courier::CHS::VERSION3);

	Server server;

	// init server
	logger.info("server.init");
	server.init("tmp/run/xns-config.json");

	// add service
	server.add(&chService2);
//	server.add(&chService3);

	// add listener
	server.add(&echoListener);
	server.add(&ripListener);
	server.add(&timeListener);
	server.add(&chsListener);
	server.add(&sppServerCourie);

	logger.info("server.start");
	server.start();
	logger.info("QThread::sleep");
	QThread::sleep(60);
	logger.info("server.stop");
	server.stop();
	logger.info("STOP testXNSServer");

	logger.info("STOP");
	return 0;
}


