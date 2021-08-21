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


class RIPListener : public XNS::Server::SocketListener {
public:
	RIPListener() : XNS::Server::SocketListener(XNS::IDP::Socket::RIP) {}

	void run() {
		threadRunning = true;
		logger.info("RIPListener START");
		logger.info("RIPListener socket = %s", XNS::IDP::Socket::toString(socket()));
		// clear list
		clear();

		{
			QMutexLocker locker(&listMutex);
			for(;;) {
				// wait timeout or entry is added to list
				for(;;) {
					bool ret = listCV.wait(&listMutex, 1000);
					if (ret) break; // listCV is notified
					if (stopThread) goto exitLoop;
				}
				// We are protected by QMutexLocker locker(&listMutex)
				if (list.isEmpty()) continue;
				Entry entry = list.takeLast();
				logger.info("RIPListener %s", entry.idp.toString());
			}
		}

exitLoop:
		logger.info("RIPListener STOP");
		threadRunning = false;
	}
};

class CHSListener : public XNS::Server::SocketListener {
public:
	CHSListener() : XNS::Server::SocketListener(XNS::IDP::Socket::CHS) {}

	void run() {
		threadRunning = true;
		logger.info("CHSListener START");
		logger.info("CHSListener socket = %s", XNS::IDP::Socket::toString(socket()));
		// clear list
		clear();

		{
			QMutexLocker locker(&listMutex);
			for(;;) {
				// wait timeout or entry is added to list
				for(;;) {
					bool ret = listCV.wait(&listMutex, 1000);
					if (ret) break; // listCV is notified
					if (stopThread) goto exitLoop;
				}
				// We are protected by QMutexLocker locker(&listMutex)
				if (list.isEmpty()) continue;
				Entry entry = list.takeLast();
				logger.info("CHSListener %s", entry.idp.toString());
			}
		}

exitLoop:
		logger.info("CHSListener STOP");
		threadRunning = false;
	}
};

void testXNSServer() {
	logger.info("START");

	XNS::Server::Server server;

	logger.info("server.init");
	server.init("tmp/run/xns-config.json");

	XNS::Server::SocketListener* ripListener = new RIPListener();
	XNS::Server::SocketListener* chsListener = new CHSListener();
	server.add(ripListener);
	server.add(chsListener);

	logger.info("server.start");
	server.start();
	logger.info("QThread::sleep");
	QThread::sleep(10);
	logger.info("server.stop");
	server.stop();
	logger.info("server.wait");
	server.wait();
	logger.info("STOP");
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


