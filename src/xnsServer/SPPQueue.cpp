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
// SPPQueue.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("spp-queue");

#include "../courier/Type.h"

#include "../xns/SPP.h"

#include "Server.h"

#include "SPPQueue.h"

using Network::Packet;
using Courier::BLOCK;
using Courier::Services;
using XNS::Server::SPPQueue;
using XNS::Server::SPPQueueServer;


void SPPQueue::init(XNS::Server::Server* server) {
	myServer = server;

	functionTable.recv         = [this](XNS::Data* data, XNS::SPP* spp){return recv(data, spp);};
	functionTable.send         = [this](XNS::Data* data, XNS::SPP* spp){return send(data, spp);};
	functionTable.close        = [this](){return close();};
	functionTable.stopRun      = [this](){return stopRun();};
	functionTable.getConfig    = [this](){return getConfig();};
	functionTable.getContext   = [this](){return getContext();};
	functionTable.getListeners = [this](){return getListeners();};
}
void SPPQueue::start() {
	if (myServer == nullptr) ERROR();

	stopFuture    = false;
	stopIsCalled  = false;
	closeIsCalled = false;
	futureSend = QtConcurrent::run([this](){this->sendThread();});
	futureRun  = QtConcurrent::run([this](){this->runThread();});
}
void SPPQueue::stop() {
	stopIsCalled = true;
	stopFuture   = true;

	futureRun.waitForFinished();
	futureSend.waitForFinished();

	// delete this at last statement
	if (closeIsCalled) {
		logger.info("delete this in stop");
		delete this;
	}
}

void SPPQueue::runThread() {
	run(functionTable);

	if (stopIsCalled) {
		// just return
		return;
	} else {
		if (closeIsCalled) {
			goto delete_this;
		} else {
			// run voluntary returns
			// just return;
			logger.debug("Unexpected");
			logger.debug("  stopFuture %s", stopFuture ? "true" : "false");
			return;
		}
	}

delete_this:
	// wait sendThread and delete this
	stopFuture = true;
	futureSend.waitForFinished();
	logger.info("delete this in runThread");
	delete this;
}
void SPPQueue::sendThread() {
	quint32 WAIT_TIME = 1;

	QMutexLocker mutexLocker(&sendListMutex);
	for(;;) {
		if (stopFuture) break;
		if (sendList.isEmpty()) {
			// wait until notified
			(void)sendListCV.wait(&sendListMutex, WAIT_TIME);
		}
		if (sendList.isEmpty()) {
			continue;
		} else {
			auto myData = sendList.takeLast();
			mutexLocker.unlock();
			Listener::transmit(myData.data, myData.spp);
			mutexLocker.relock();
		}
	}
}

void SPPQueue::handle(const Data& data, const SPP& spp) {
	MyData myData;
	myData.data = data;
	myData.spp  = spp;

	QString timeStamp = QDateTime::fromMSecsSinceEpoch(myData.data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(myData.data.ethernet.toString()), TO_CSTRING(myData.data.idp.toString()));
	logger.info("%s  SPP   %s  HANDLE", TO_CSTRING(header), TO_CSTRING(myData.spp.toString()));

	recvListMutex.lock();
	recvList.prepend(myData);
	recvListMutex.unlock();
	recvListCV.wakeOne();
}


bool SPPQueue::recv(Data* data, SPP* spp) {
	quint32 WAIT_TIME = 1;

	QMutexLocker mutexLocker(&recvListMutex);
	if (recvList.isEmpty()) {
		// wait until notified
		(void)recvListCV.wait(&recvListMutex, WAIT_TIME);
	}
	if (recvList.isEmpty()) {
		return false;
	} else {
		auto myData = recvList.takeLast();
		*data = myData.data;
		*spp  = myData.spp;
		return true;
	}
}
void SPPQueue::send(Data* data, SPP* spp) {
	MyData myData;
	myData.data = *data;
	myData.spp  = *spp;

	sendListMutex.lock();
	sendList.prepend(myData);
	sendListMutex.unlock();
	sendListCV.wakeOne();
}
void SPPQueue::close() {
	Listeners* listeners = myServer->getListeners();
	listeners->remove(socket());
	closeIsCalled = true;
	stopFuture    = true;
}
bool SPPQueue::stopRun() {
	return stopFuture;
}
XNS::Config*            SPPQueue::getConfig() {
	return myServer->getConfig();
}
XNS::Context*           SPPQueue::getContext() {
	return myServer->getContext();
}
XNS::Server::Listeners* SPPQueue::getListeners() {
	return myServer->getListeners();
}


//
// SPPQueueServer
//
void SPPQueueServer::start() {
	if (myServer == nullptr) ERROR();
}
void SPPQueueServer::handle(const Data& data, const SPP& spp) {
	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  SPP   %s  SPPQueueServer", TO_CSTRING(header), TO_CSTRING(spp.toString()));

	if (spp.control.isSystem() && spp.control.isSendAck()) {
		// OK
		SPPQueue::State state;
		XNS::Server::Listeners* listeners = myServer->getListeners();

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

			state.recvSST      = SPP::SST::DATA;

			// first reply packet is system, so sequence number is still 0
			state.recvSeq      = 0;
			state.sendSeq      = 0;
		}

		// create listener object and add to listeners
		{
			SPPQueue* newImpl = myImpl->clone();
			newImpl->socket(state.localSocket);
			newImpl->name(state.name);
			newImpl->autoDelete(true);
			newImpl->state(state);
			// start listening object
			// newImpl.start() is called during listeners->add()
			listeners->add(newImpl);
			newImpl->start();
		}

		// Send reply packet
		{
			SPP reply;
			reply.control = SPP::Control::BIT_SYSTEM;
			reply.sst = SPP::SST::DATA;
			reply.idSrc = state.localID;
			reply.idDst = state.remoteID;
			reply.seq   = state.sendSeq;
			reply.ack   = state.recvSeq;
			reply.alloc = state.recvSeq;

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
