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
static const util::Logger logger(__FILE__);

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
// SPPQueue
//
SPPQueue::SPPQueue(const char* name, uint16_t socket) : SPPListener(name, socket) {
	myServer     = nullptr;
	recvListSeq  = 0;
	time         = 0;
	remoteHost   = 0;
	remoteSocket = 0;
	remoteID     = 0;
	localSocket  = 0;
	localID      = 0;
	sendSeq      = 0;
	recvSeq      = 0;
	driver       = nullptr;
	localNet     = 0;
	localHost    = 0;
}
SPPQueue::SPPQueue(const SPPQueue& that)             : SPPListener(that) {
	myServer     = nullptr;
	recvListSeq  = 0;
	time         = 0;
	remoteHost   = 0;
	remoteSocket = 0;
	remoteID     = 0;
	localSocket  = 0;
	localID      = 0;
	sendSeq      = 0;
	recvSeq      = 0;
	driver       = nullptr;
	localNet     = 0;
	localHost    = 0;
}

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

	// clear transmitList
	transmitList.clear();

	stopFuture     = 0;
	stopIsCalled   = 0;
	closeIsCalled  = 0;
	futureTransmit = QtConcurrent::run([this](){this->transmitThread();});
	futureSend     = QtConcurrent::run([this](){this->sendThread();});
	futureRun      = QtConcurrent::run([this](){this->runThread();});
}
void SPPQueue::stop() {
	stopIsCalled = 1;
	stopFuture   = 1;

	futureRun.waitForFinished();
	futureSend.waitForFinished();
	futureTransmit.waitForFinished();

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
		std::string timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		std::string header = std::string::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
		logger.info("%s  SPP   %s  HANDLE  %s!", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(spp.block.toString()));
	}

	// sanity check
	if (remoteHost != data.idp.srcHost || remoteSocket != data.idp.srcSocket || remoteID != spp.idSrc) {
		// something goes wrong
		logger.error("Unexpected");
		logger.error("  expect  %04X  %s-%s", remoteID,           TO_CSTRING(Host::toString(remoteHost)),  TO_CSTRING(Socket::toString(remoteSocket)));
		logger.error("  actual  %04X  %s-%s", (uint16_t)spp.idSrc, TO_CSTRING(data.idp.srcHost.toString()), TO_CSTRING(data.idp.srcSocket.toString()));
		ERROR();
	}

	logger.info("recvBuffer %s", recvBuffer.toString());
	logger.info("recvSeq %3d  sendSeq %3d", (uint16_t)recvSeq, (uint16_t)sendSeq);

	// maintain myState
	//   if accept this packet, increment recvSeq
	bool needToSendAck = false;

	if (spp.control.isSendAck()) {
		needToSendAck = true;
	}

	//
	// update recvBuffer
	//
	{
		bool ignorePacket  = false;
		if (spp.control.isData()) {
			// process data and update recvBuffer
			Buffer::Entry* entry = recvBuffer.get(spp.seq);
			if (entry == nullptr) {
				// spp.seq is out of bounds of recvBuffer
				// this packet is error
				// ignore this packet
				logger.warn("Unexpected");
				logger.warn("  spp        %s", spp.toString());
				logger.warn("  recvBuffer %s", recvBuffer.toString());
				ignorePacket = true;
			} else {
				if (entry->inUse()) {
					// spp.seq is in recvBuffer and data is already received
					// this packet is duplicate
					// ignore this packet
					logger.warn("Duplicate");
					logger.warn("  spp        %s", spp.toString());
					logger.warn("  recvBuffer %s", recvBuffer.toString());
					ignorePacket = true;
				} else {
					// spp.seq is in recvBuffer and data is not received
					// this packet is new
					// accept this packet and set as data in recvBuffer
					QueueData* myData = new QueueData(data, spp);
					entry->myData = myData;
				}
			}
		}
		if (ignorePacket) return;
	}

	//
	// update recvList with recvList
	//
	if (spp.control.isData()) {
		// move data from recvBuffer to recvList if possible
		// To move data in sequential, we are using recvListSeq
		// recvListSeq is next seq to added to recvList
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
	}

	//
	// update recvSeq and recvBuffer
	//
	// spp.seq in system packet is for next data packet. so process only data packet
	if (spp.control.isData()) {
		// If we accept next expecting data packet, update recvSeq for spp.ack
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

	// update sendSeq
	{
		uint16_t nextSendSeq = sendSeq + 1;
		if (spp.alloc == nextSendSeq) {
			// other end acknowledge sendSeq, increment sendSeq
			// free current
			sendBufferMutex.lock();
			sendBuffer.free(sendSeq);
			sendSeq++;
			sendBufferMutex.unlock();
			// state is changed. need notify new state to other end.
			needToSendAck = true;
		}
	}

	if (needToSendAck) sendAck(data);
}


//
// Send
//
void SPPQueue::sendThread() {
	// retransmit data in sendBuffer in every WAIT_TIME
	uint32_t WAIT_TIME = 500; // unit is msec

	QMutexLocker mutexLocker(&sendBufferMutex);
	for(;;) {
		if (stopFuture) break;
		if (sendBuffer.isEmpty()) {
			// wait until notified
			(void)sendBufferCV.wait(&sendBufferMutex, WAIT_TIME);
		}
		if (sendBuffer.isEmpty()) {
			continue;
		} else {
			uint16_t seq = sendSeq;
			for(;;) {
				Buffer::Entry* entry = sendBuffer.get(seq);
				if (entry == nullptr) break;
				transmit(entry->myData);
				seq++;
			}
		}
	}
}
void SPPQueue::sendAck(const Data& data) {
	// spp.block don't share with data.packet
	// no need to fix spp.block
	SPP spp;
	spp.control = SPP::Control::BIT_SYSTEM;
	spp.sst     = SPP::SST::DATA;
	spp.idSrc   = localID;
	spp.idDst   = remoteID;
	spp.seq     = sendSeq;
	spp.ack     = recvSeq;
	spp.alloc   = recvSeq + recvBuffer.countFree();

	QueueData* myData = new QueueData(data, spp);
	transmit(myData);
}


//
// transmit
//
void SPPQueue::transmit(QueueData* myData) {
	transmitListMutex.lock();
	transmitList.append(myData);
	transmitListMutex.unlock();
	transmitListCV.wakeOne();
}
void SPPQueue::transmitThread() {
	uint32_t WAIT_TIME = 1000; // unit is msec
	QMutexLocker mutexLocker(&transmitListMutex);

	for(;;) {
		if (stopFuture) break;

		if (transmitList.isEmpty()) {
			// wait until notified
			(void)transmitListCV.wait(&transmitListMutex, WAIT_TIME);
		}
		if (transmitList.isEmpty()) {
			continue;
		} else {
			// transmit all in transmitList
			for(auto e: transmitList) {
				Data& data(e->data);
				SPP&  spp (e->spp);

				Packet level2;
				TO_BYTE_BUFFER(level2, spp);
				BLOCK block(level2);

				IDP idp;
				idp.checksum_ = data.idp.checksum_;
				idp.length    = (uint16_t)0;
				idp.control   = (uint8_t)0;
				idp.type      = IDP::Type::SPP;
				idp.dstNet    = data.idp.srcNet;
				idp.dstHost   = remoteHost;
				idp.dstSocket = remoteSocket;
				idp.srcNet    = localNet;
				idp.srcHost   = localHost;
				idp.srcSocket = socket();
				idp.block     = block;

				Listener::transmit(driver, data.ethernet.src, localHost, idp);

				delete e;
			}
			transmitList.clear();
		}
	}
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
	futureTransmit.waitForFinished();
	logger.info("delete this in runThread  %s", toString());
	delete this;
}

SPPQueue::QueueData* SPPQueue::recv() {
	uint32_t WAIT_TIME = 1000; // unit is msec

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

	sendBufferMutex.lock();
	Buffer::Entry* entry = sendBuffer.alloc(spp.seq);
	entry->myData = myData;
	sendBufferMutex.unlock();

	sendBufferCV.wakeOne();
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
void SPPQueue::allocNext(uint16_t seq) {
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
std::string SPPQueue::Buffer::Entry::toString() {
	return std::string("(%1 %2)").arg(seq).arg(myData == nullptr ? "notInUse" : "inUse");
}
//
// SPPQueue::Buffer
//
SPPQueue::Buffer::~Buffer() {
	clear();
}
SPPQueue::Buffer::Entry* SPPQueue::Buffer::get(uint16_t seq) {
	return map.contains(seq) ? map[seq] : nullptr;
}
SPPQueue::Buffer::Entry* SPPQueue::Buffer::alloc(uint16_t seq) {
	if (map.contains(seq)) {
		logger.error("Unexpected");
		logger.error("  seq    %d", seq);
		logger.error("  buffer %s", toString());
		ERROR();
	} else {
		Entry* ret = new Entry(seq);
		map[seq] = ret;
		return ret;
	}
}
void SPPQueue::Buffer::free(uint16_t seq) {
	auto i = map.find(seq);
	if (i == map.end()) {
		logger.error("Unexpected");
		logger.error("  seq    %d", seq);
		logger.error("  buffer %s", toString());
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
std::string SPPQueue::Buffer::toString() {
	std::stringList list;
	for(auto e: map.values()) {
		list += e->toString();
	}
	return std::string("(%1)").arg(list.join(", "));
}
void SPPQueue::set(const SPPQueueServer::Context& context) {
	newName      = context.newName;
	time         = context.time;
	remoteHost   = context.remoteHost;
	remoteSocket = context.remoteSocket;
	remoteID     = context.remoteID;
	localSocket  = context.localSocket;
	localID      = context.localID;
}


//
// SPPQueueServer
//
SPPQueueServer::SPPQueueServer(SPPQueue* impl) : SPPListener(impl->name(), impl->socket()), myImpl(impl), myServer(nullptr) {}

void SPPQueueServer::start() {
	if (myServer == nullptr) ERROR();
}
void SPPQueueServer::handle(const Data& data, const SPP& spp) {
	std::string timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	std::string header = std::string::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  SPP   %s  SPPQueueServer", TO_CSTRING(header), TO_CSTRING(spp.toString()));

	if (spp.control.isSystem() && spp.control.isSendAck()) {
		Listeners* listeners = myServer->getListeners();

		// build state
		Context context;
		{
			std::string tempName = std::string("%1-client").arg(name());

			context.newName      = tempName.toUtf8().constData();
			context.time         = data.timeStamp;

			context.remoteHost   = data.idp.srcHost;
			context.remoteSocket = (uint16_t)data.idp.srcSocket;
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
