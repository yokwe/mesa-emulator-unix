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
	XNS::Config config = XNS::loadConfig(path);

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

	logger.info("device   = %s  %s", XNS::IDP::Host::toHexaDecimalString(device.address, ":"), device.name);
	logger.info("localNet = %d", localNet);
}


//
// XNS::Server::Server
//
void XNS::Server::Server::init(const QString& path) {
	processThreadPool->setObjectName("ProcessThereadPool");
	processThreadPool->setMaxThreadCount(1);

	listenerThreadPool->setObjectName("ListenerThereadPool");

	context = Context(path);
	processThread = new ProcessThread(context, listenerMap, listenerThreadPool);
	processThread->setAutoDelete(false);
}
void XNS::Server::Server::add(SocketListener* listener) {
	quint16 socket = listener->socket();
	logger.info("add socket listener %s", XNS::IDP::Socket::toString(socket));
	listener->setAutoDelete(false); // make sure don't delete listener after done
	listenerMap[socket] = listener;
}
bool XNS::Server::Server::running() {
	return processThread->running();
}
void XNS::Server::Server::start() {
	if (processThread->running()) {
		logger.error("Unexpected");
		ERROR()
	} else {
		// start processThread
		logger.info("processThread START");
		processThreadPool->start(processThread);
	}
}
void XNS::Server::Server::stop() {
	if (processThread->running()) {
		logger.info("processThread STOP");
		processThread->stop();
	} else {
		logger.warn("processThread already stop");
	}
}
void XNS::Server::Server::wait() {
	if (processThread->running()) {
		logger.info("processThread WAIT");
		processThreadPool->waitForDone();
		logger.info("processThread DONE");

		logger.info("listenerThreadPool WAIT");
		listenerThreadPool->waitForDone();
		logger.info("listenerThreadPool DONE");

	} else {
		logger.warn("processThread already done");
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

	// start listener thread
	{
		int listenerCount = listenerMap.size();
		logger.info("listenerCount %d", listenerCount);
		threadPool->setMaxThreadCount(listenerCount);

		for(auto e: listenerMap.keys()) {
			int socket = e;
			SocketListener* listener = listenerMap[socket];
			threadPool->start(listener);
		}
	}

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

			XNS::Ethernet ethernet;
			FROM_BYTE_BUFFER(packet, ethernet);
			ByteBuffer::Buffer level1 = ethernet.block.toBuffer();

			if (ethernet.type != XNS::Ethernet::Type::XNS) continue;

			XNS::IDP idp;
			FROM_BYTE_BUFFER(level1, idp);

			logger.info("%s", ethernet.toString());
			logger.info("    %s", idp.toString());

			quint16 socket = (quint16)idp.dstSocket;

			if (listenerMap.contains(socket)) {
				SocketListener::Entry entry(context, packet, ethernet, idp);
				SocketListener* socketListener = listenerMap[socket];
				socketListener->add(entry);
			} else {
				logger.warn("no handler for socket %s", XNS::IDP::Socket::toString(socket));
			}
		}
exitLoop:
		logger.info("ProcessThread run stop");
	}

	threadRunning = false;
}
void XNS::Server::ProcessThread::stop() {
	if (running()) {
		stopThread = true;

		// stop listener
		for(auto e: listenerMap.keys()) {
			int socket = e;
			logger.info("stop listener %s", XNS::IDP::Socket::toString(socket));
			SocketListener* listener = listenerMap[socket];
			if (listener->running()) {
				listener->stop();
			}
		}

	} else {
		logger.warn("processThread already stop");
	}
}


//
// XNS::Server::SocketListener
//
void XNS::Server::SocketListener::add(const Entry& entry) {
	QMutexLocker locker(&listMutex);
	list.append(entry);
	listCV.wakeOne();
}
XNS::Server::SocketListener::Entry XNS::Server::SocketListener::get() {
	QMutexLocker locker(&listMutex);
	if (list.isEmpty()) {
		logger.error("Unexpected");
		ERROR();
	}
	return list.takeLast();
}
bool XNS::Server::SocketListener::isEmpty() {
	QMutexLocker locker(&listMutex);
	maintainList();
	return list.isEmpty();
}
void XNS::Server::SocketListener::maintainList() {
	quint64 limit = QDateTime::currentSecsSinceEpoch() - XNS::IDP::MPL;
	for(;;) {
		if (list.isEmpty()) break;
		Entry& last = list.last();
		if (last.time < limit) {
			list.removeLast();
		} else {
			break;
		}
	}
}
void XNS::Server::SocketListener::clear() {
	QMutexLocker locker(&listMutex);
	list.clear();
}
bool XNS::Server::SocketListener::running() {
	return threadRunning;
}
void XNS::Server::SocketListener::stop() {
	logger.info("SocketListener stop %s", XNS::IDP::Socket::toString(socket()));
	if (running()) {
		stopThread = true;
	} else {
		logger.warn("socketListener already stop");
	}
}



//class RIPListener : public XNS::Server::SocketListener {
//public:
//	RIPListener() : XNS::Server::SocketListener(XNS::IDP::Socket::RIP) {}
//
//	void run() {
//		threadRunning = true;
//		logger.info("RIPListener START");
//		logger.info("RIPListener socket = %s", XNS::IDP::Socket::toString(socket()));
//		// clear list
//		clear();
//
//		{
//			QMutexLocker locker(&listMutex);
//			for(;;) {
//				// wait timeout or entry is added to list
//				for(;;) {
//					bool ret = listCV.wait(&listMutex, 1000);
//					if (ret) break; // listCV is notified
//					if (stopThread) goto exitLoop;
//				}
//				// We are protected by QMutexLocker locker(&listMutex)
//				if (list.isEmpty()) continue;
//				Entry entry = list.takeLast();
//				logger.info("RIPListener %s", entry.idp.toString());
//			}
//		}
//
//exitLoop:
//		logger.info("RIPListener STOP");
//		threadRunning = false;
//	}
//};

