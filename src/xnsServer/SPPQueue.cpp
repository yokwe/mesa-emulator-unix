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


//
// Life Cycle
//
void SPPQueue::init(XNS::Server::Server* server) {
	myServer  = server;
	driver    = myServer->getContext()->driver;
	localNet  = myServer->getConfig()->local.net;
	localHost = myServer->getConfig()->local.host;

	functionTable.recv         = [this](RecvData* redvData){return recv(redvData);};
	functionTable.send         = [this](XNS::Data* data, XNS::SPP* spp){return send(data, spp);};
	functionTable.close        = [this](){return close();};
	functionTable.stopRun      = [this](){return stopRun();};
	functionTable.getConfig    = [this](){return getConfig();};
	functionTable.getContext   = [this](){return getContext();};
	functionTable.getListeners = [this](){return getListeners();};
}
void SPPQueue::start() {
	if (myServer == nullptr) ERROR();

	// FIXME how to initialize recvBuffer

	stopFuture    = 0;
	stopIsCalled  = 0;
	closeIsCalled = 0;
	futureSend = QtConcurrent::run([this](){this->sendThread();});
	futureRun  = QtConcurrent::run([this](){this->runThread();});
}
void SPPQueue::stop() {
	stopIsCalled = 1;
	stopFuture   = 1;

	futureRun.waitForFinished();
	futureSend.waitForFinished();

	// delete this at last statement
	if (closeIsCalled) {
		logger.info("delete this in stop  %s", toString());
		delete this;
	}
}


//
// process revived data
//
void SPPQueue::handle(const Data& data, const SPP& spp) {
	RecvData myData(data, spp);

	QString timeStamp = QDateTime::fromMSecsSinceEpoch(myData.data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(myData.data.ethernet.toString()), TO_CSTRING(myData.data.idp.toString()));
	logger.info("%s  SPP   %s  HANDLE  %s!", TO_CSTRING(header), TO_CSTRING(myData.spp.toString()), TO_CSTRING(myData.spp.block.toString()));

	// sanity check
	if (myState.remoteHost != data.idp.srcHost || myState.remoteSocket != data.idp.srcSocket || myState.remoteID != spp.idSrc) {
		// something goes wrong
		logger.error("Unexpected");
		logger.error("  expect  %04X  %s-%s", myState.remoteID,   TO_CSTRING(Host::toString(myState.remoteHost)), TO_CSTRING(Socket::toString(myState.remoteSocket)));
		logger.error("  actual  %04X  %s-%s", (quint16)spp.idSrc, TO_CSTRING(data.idp.srcHost.toString()),        TO_CSTRING(data.idp.srcSocket.toString()));
		ERROR();
	}

	// maintain myState
	//   if accept this packet, increment recvSeq
	bool reject        = false;
	bool needToSendAck = false;
	{
		if (spp.control.isSystem()) {
			// system packet
		} else {
			// data packet
			if (spp.seq == recvSeq) {
				// accept this packet
				// advance recvSeq
				// If recvSeq overflow, recvSeq become ZERO
				recvSeq++;
				// need to notify other end
				needToSendAck = true;
			} else {
				// unexpected seq number
				logger.warn("Unexpected seq");
				logger.warn("  expect %d", recvSeq);
				logger.warn("  actual %d", (quint16)spp.seq);
				reject = true;
			}
		}
		if (spp.alloc == sendSeq) {
			// no change
		} else {
			quint16 nextSendSeq = sendSeq + 1;
			if (spp.alloc == nextSendSeq) {
				// advance
				sendSeq++;
				needToSendAck = true;
			}
		}

	}
	if (reject) return;

	recvListMutex.lock();
	recvList.prepend(myData);
	recvListMutex.unlock();
	recvListCV.wakeOne();

	if (needToSendAck) {
		sendAck(data);
	}
}


//
// Send
//
void SPPQueue::sendAck(const Data& data) {
	SPP spp;
	spp.control = SPP::Control::BIT_SYSTEM;
	spp.sst     = SPP::SST::DATA;
	spp.idSrc   = myState.localID;
	spp.idDst   = myState.remoteID;
	spp.seq     = sendSeq;
	spp.ack     = recvSeq;
	spp.alloc   = recvSeq + recvBuffer.countEmpty();

	send(&data, &spp);
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
			transmit(myData.data, myData.spp);
			mutexLocker.relock();
		}
	}
}
void SPPQueue::transmit(const Data& data, const SPP& spp) {
	Packet level2;
	TO_BYTE_BUFFER(level2, spp);
	BLOCK block(level2);

	IDP idp;
	idp.checksum_ = data.idp.checksum_;
	idp.length    = (quint16)0;
	idp.control   = (quint8)0;
	idp.type      = IDP::Type::SPP;
	idp.dstNet    = data.idp.srcNet;
	idp.dstHost   = myState.remoteHost;
	idp.dstSocket = myState.remoteSocket;
	idp.srcNet    = localNet;
	idp.srcHost   = localHost;
	idp.srcSocket = socket();
	idp.block     = block;

	// Use ethernet.src for destination. not idp.srcHost
	// Don't mix with ethernet.src and idp.srcHost
	Listener::transmit(driver, data.ethernet.src, localHost, idp);
}


//
// Run
//
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
	stopFuture = 1;
	futureSend.waitForFinished();
	logger.info("delete this in runThread  %s", toString());
	delete this;
}

// IMPORTAN
//   Need to be pass one RedvData. Because pss is using data.packet.
//   Don't break RecvData as Data and SPP
bool SPPQueue::recv(RecvData* recvData) {
	quint32 WAIT_TIME = 1;

	QMutexLocker mutexLocker(&recvListMutex);
	if (recvList.isEmpty()) {
		// wait until notified
		(void)recvListCV.wait(&recvListMutex, WAIT_TIME);
	}
	if (recvList.isEmpty()) {
		return false;
	} else {
		*recvData = recvList.takeLast();
		return true;
	}
}
void SPPQueue::send(const Data* data, const SPP* spp) {
	RecvData myData(*data, *spp);

	sendListMutex.lock();
	sendList.prepend(myData);
	sendListMutex.unlock();
	sendListCV.wakeOne();
}
void SPPQueue::close() {
	logger.info("SPPQueue::close  remove listener %s", toString());
	Listeners* listeners = myServer->getListeners();
	listeners->remove(socket());
	closeIsCalled = 1;
	stopFuture    = 1;
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
		Listeners* listeners = myServer->getListeners();

		// build state
		SPPQueue::State state;
		{
			QString newName = QString("%1-client").arg(name());

			state.name         = newName.toUtf8().constData();
			state.time         = data.timeStamp;

			state.remoteHost   = data.idp.srcHost;
			state.remoteSocket = (quint16)data.idp.srcSocket;
			state.remoteID     = spp.idSrc;

			state.localSocket  = listeners->getUnusedSocket();
			state.localID      = data.timeStamp / 100;
		}

		// build listener newImpl with clone of myImpl
		SPPQueue* newImpl = myImpl->clone();
		{
			newImpl->socket(state.localSocket);
			newImpl->name(state.name.constData());
			newImpl->autoDelete(true);
			newImpl->state(state);
		}

		// add to listeners
		listeners->add(newImpl);
		// start listening
		newImpl->startListener();
		// send ack packet
		newImpl->sendAck(data);
	} else {
		logger.error("Unexpected");
		ERROR();
	}
}
