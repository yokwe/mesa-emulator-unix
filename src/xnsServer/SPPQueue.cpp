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
using XNS::Data;
using XNS::SPP;
using XNS::Server::SPPListener;
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

	functionTable.recv         = [this](){return recv();};
	functionTable.send         = [this](Data& data, SPP& spp){return send(data, spp);};
	functionTable.close        = [this](){return close();};
	functionTable.stopRun      = [this](){return stopRun();};
	functionTable.getConfig    = [this](){return getConfig();};
	functionTable.getContext   = [this](){return getContext();};
	functionTable.getListeners = [this](){return getListeners();};
	functionTable.getServices  = [this](){return getServices();};
}
void SPPQueue::start() {
	if (myServer == nullptr) ERROR();

	// clear recvBuffer
	recvBuffer.clear();
	// allocate 4 entries
	recvBuffer.alloc(0);
	recvBuffer.alloc(1);
	recvBuffer.alloc(2);
	recvBuffer.alloc(3);

	// clear recvList
	recvList.clear();
	recvListSeq = 0;

	// clear sendBuffer
	sendBuffer.clear();

	stopFuture    = 0;
	stopIsCalled  = 0;
	closeIsCalled = 0;
	futureSend    = QtConcurrent::run([this](){this->sendThread();});
	futureRun     = QtConcurrent::run([this](){this->runThread();});
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
// process received data
//
void SPPQueue::handle(const Data& data, const SPP& spp) {
	{
		QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  SPP   %s  HANDLE  %s!", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(spp.block.toString()));
	}

	// sanity check
	if (remoteHost != data.idp.srcHost || remoteSocket != data.idp.srcSocket || remoteID != spp.idSrc) {
		// something goes wrong
		logger.error("Unexpected");
		logger.error("  expect  %04X  %s-%s", remoteID,           TO_CSTRING(Host::toString(remoteHost)),  TO_CSTRING(Socket::toString(remoteSocket)));
		logger.error("  actual  %04X  %s-%s", (quint16)spp.idSrc, TO_CSTRING(data.idp.srcHost.toString()), TO_CSTRING(data.idp.srcSocket.toString()));
		ERROR();
	}

	logger.info("recvBuffer %s", recvBuffer.toString());
	logger.info("recvSeq %3d  sendSeq %3d", recvSeq, sendSeq);

	// maintain myState
	//   if accept this packet, increment recvSeq
	bool reject        = false;
	bool needToSendAck = false;

	{
		if (spp.control.isSendAck()) {
			needToSendAck = true;
		}

		if (spp.control.isData()) {
			// process data and update recvBuffer
			Buffer::Entry* entry = recvBuffer.get(spp.seq);
			if (entry == nullptr) {
				logger.warn("Unexpected");
				logger.warn("  spp        %s", spp.toString());
				logger.warn("  recvBuffer %s", recvBuffer.toString());
				reject = true;
			} else {
				if (entry->inUse()) {
					// already in buffer
					reject = true;
				} else {
					// new entry
					QueueData* myData = new QueueData(data, spp);
					entry->myData = myData;
				}
			}
		}
	}

	if (reject) return;

	if (spp.control.isData()) {
		// add recvList as many as possible in order by recvListSeq
		bool needsToWakeOne = false;
		for(;;) {
			Buffer::Entry* entry = recvBuffer.get(recvListSeq);
			if (entry == nullptr) break;
			if (!entry->inUse()) break;

			recvListMutex.lock();
			recvList.prepend(entry->myData);
			recvListMutex.unlock();
			needsToWakeOne = true;

			// increment recvListSeqs
			recvListSeq++;
		}
		if (needsToWakeOne) recvListCV.wakeOne();

		// check spp.seq if data packet
		if (spp.seq == recvSeq) {
			// other end acknowledge recvSeq, increment recvSeq
			// free current
			recvBuffer.free(recvSeq);
			recvSeq++;
			// alloc next
			allocNext(recvSeq);
			// state is changed. need notify new state to other end.
			needToSendAck = true;
		}
	}

	// check spp.alloc
	{
		quint16 nextSendSeq = sendSeq + 1;
		if (spp.alloc == nextSendSeq) {
			// other end acknowledge sendSeq, increment sendSeq
			sendSeq++;
			// state is changed. need notify new state to other end.
			needToSendAck = true;
		}
	}

	if (needToSendAck) sendAck(data);
}


//
// Send
//
void SPPQueue::sendAck(const Data& data) {
	SPP spp;
	spp.control = SPP::Control::BIT_SYSTEM;
	spp.sst     = SPP::SST::DATA;
	spp.idSrc   = localID;
	spp.idDst   = remoteID;
	spp.seq     = sendSeq;
	spp.ack     = recvSeq;
	spp.alloc   = recvSeq + recvBuffer.countFree();

	send(data, spp);
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
			QueueData* myData = sendList.takeLast();
			mutexLocker.unlock();
			transmit(myData->data, myData->spp);
			mutexLocker.relock();
			delete myData;
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
	idp.dstHost   = remoteHost;
	idp.dstSocket = remoteSocket;
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

SPPQueue::QueueData* SPPQueue::recv() {
	quint32 WAIT_TIME = 1;

	QMutexLocker mutexLocker(&recvListMutex);
	if (recvList.isEmpty()) {
		// wait until notified
		(void)recvListCV.wait(&recvListMutex, WAIT_TIME);
	}
	if (recvList.isEmpty()) {
		return nullptr;
	} else {
		return recvList.takeLast();
	}
}
void SPPQueue::send(const Data& data, const SPP& spp) {
	QueueData* myData = new QueueData(data, spp);

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
XNS::Server::Services* SPPQueue::getServices() {
	return myServer->getServices();
}
void SPPQueue::allocNext(quint16 seq) {
	for(;;) {
		Buffer::Entry* p = recvBuffer.get(seq);
		if (p == nullptr) {
			recvBuffer.alloc(seq);
			break;
		}
		seq++;
	}
}


//
// SPPQueue::QueueData
//
void SPPQueue::QueueData::copyFrom(const Data& data_, const SPP& spp_) {
	data = data_;
	spp  = spp_;
}
void SPPQueue::QueueData::fixBlock() {
	// reflect change of address of data.packet
	BLOCK newValue(data.packet);
	data.ethernet.updateBlock(newValue);
	data.idp.block.updateBufferData(newValue);
	spp.updateBlock(newValue);
}
SPPQueue::QueueData::QueueData() {
	// reflect change of address of data.packet
	BLOCK newValue(data.packet);
	spp.updateBlock(newValue);
}
SPPQueue::QueueData::~QueueData() {
	//
}
SPPQueue::QueueData::QueueData(const QueueData& that) : QueueData(that.data, that.spp) {}
SPPQueue::QueueData& SPPQueue::QueueData::operator = (const SPPQueue::QueueData& that) {
	copyFrom(that.data, that.spp);
	fixBlock();
	return *this;
}
SPPQueue::QueueData::QueueData(const Data& data_, const SPP& spp_) {
	copyFrom(data_, spp_);
	fixBlock();
}


//
// SPPQueue::Buffer::Entry
//
QString SPPQueue::Buffer::Entry::toString() {
	return QString("(%1 %2)").arg(seq).arg(myData == nullptr ? "notInUse" : "inUse");
}
//
// SPPQueue::Buffer
//
SPPQueue::Buffer::~Buffer() {
	clear();
}
SPPQueue::Buffer::Entry* SPPQueue::Buffer::get(quint16 seq) {
	return map.contains(seq) ? map[seq] : nullptr;
}
SPPQueue::Buffer::Entry* SPPQueue::Buffer::alloc(quint16 seq) {
	if (map.contains(seq)) {
		logger.error("Unexpected");
		logger.error("  seq %d", seq);
		ERROR();
	} else {
		Entry* ret = new Entry(seq);
		map[seq] = ret;
		return ret;
	}
}
void SPPQueue::Buffer::free(quint16 seq) {
	auto i = map.find(seq);
	if (i == map.end()) {
		logger.error("Unexpected");
		logger.error("  seq %d", seq);
		ERROR();
	}
	map.erase(i);
}
void SPPQueue::Buffer::clear() {
	// delete element of map
	for(auto e: map.values()) {
		delete e;
	}
	// clear map
	map.clear();
}
int SPPQueue::Buffer::countFree() {
	int ret = 0;
	for(Entry* e: map.values()) {
		if (!e->inUse()) ret++;
	}
	return ret;
}
QString SPPQueue::Buffer::toString() {
	QStringList list;
	for(auto e: map.values()) {
		list += e->toString();
	}
	return QString("(%1)").arg(list.join(", "));
}

//
// SPPQueueServer
//
SPPQueueServer::SPPQueueServer(SPPQueue* impl) : SPPListener(impl->name(), impl->socket()), myImpl(impl), myServer(nullptr) {}

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
		Context context;
		{
			QString tempName = QString("%1-client").arg(name());

			context.newName      = tempName.toUtf8().constData();
			context.time         = data.timeStamp;

			context.remoteHost   = data.idp.srcHost;
			context.remoteSocket = (quint16)data.idp.srcSocket;
			context.remoteID     = spp.idSrc;

			context.localSocket  = listeners->getUnusedSocket();
			context.localID      = data.timeStamp / 100;
		}

		// build listener newImpl with clone of myImpl
		SPPQueue* newImpl = myImpl->clone();
		{
			newImpl->autoDelete(true);
			newImpl->socket(context.localSocket);
			newImpl->name(context.newName.constData());
			newImpl->set(context);
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
