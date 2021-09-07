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

#include "SPPQueue.h"

using Network::Packet;
using XNS::Data;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::SPP;
using Courier::BLOCK;
using Courier::Services;


std::function<bool(XNS::Data*, XNS::SPP*)>   getData;
std::function<bool(void)>                    stopRun;
std::function<XNS::Config*(void)>            getConfig;
std::function<XNS::Context*(void)>           getContext;
std::function<XNS::Server::Listeners*(void)> getListeners;

SPPQueue::SPPQueue(const char* name, quint16 socket) : SPPListener(name, socket) {
	stopFuture = false;
	functionTable.getData      = [this](XNS::Data* data, XNS::SPP* spp){return getData(data, spp);};
	functionTable.stopRun      = [this](){return stopFuture;};
	functionTable.getConfig    = [this](){return config;};
	functionTable.getContext   = [this](){return context;};
	functionTable.getListeners = [this](){return listeners;};
}
SPPQueue::SPPQueue(const SPPQueue& that) : SPPListener(that) {
	stopFuture = false;
	functionTable.getData      = [this](XNS::Data* data, XNS::SPP* spp){return getData(data, spp);};
	functionTable.stopRun      = [this](){return stopFuture;};
	functionTable.getConfig    = [this](){return config;};
	functionTable.getContext   = [this](){return context;};
	functionTable.getListeners = [this](){return listeners;};
}


void SPPQueue::init() {
	logger.info("init");
}
void SPPQueue::start() {
	DEBUG_TRACE();
	DefaultListener::start();
	stopFuture = false;

	future = QtConcurrent::run([this](){this->run(functionTable);});
}
void SPPQueue::stop() {
	DEBUG_TRACE();
	DefaultListener::stop();
	stopFuture = true;
	future.waitForFinished();
}

void SPPQueue::handle(const Data& data, const SPP& spp) {
	dataListMutex.lock();
	MyData myData;
	myData.data = data;
	myData.spp  = spp;

	QString timeStamp = QDateTime::fromMSecsSinceEpoch(myData.data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(myData.data.ethernet.toString()), TO_CSTRING(myData.data.idp.toString()));
	logger.info("%s  SPP   %s  HANDLE", TO_CSTRING(header), TO_CSTRING(myData.spp.toString()));

	dataList.append(myData);
	dataListMutex.unlock();
	dataListCV.wakeOne();
}


bool SPPQueue::getData(Data* data, SPP* spp) {
	quint32 WAIT_TIME = 1;

	QMutexLocker mutexLocker(&dataListMutex);
	if (dataList.isEmpty()) {
		// wait until notified
		(void)dataListCV.wait(&dataListMutex, WAIT_TIME);
	}
	if (dataList.isEmpty()) {
		return false;
	} else {
		auto myData = dataList.takeLast();
		*data = myData.data;
		*spp  = myData.spp;
		return true;
	}
}
bool SPPQueue::stopRun() {
	return stopFuture;
}
XNS::Config*            SPPQueue::getConfig() {
	return config;
}
XNS::Context*           SPPQueue::getContext() {
	return context;
}
XNS::Server::Listeners* SPPQueue::getListeners() {
	return listeners;
}


//
// SPPQueueServer
//
void SPPQueueServer::handle(const XNS::Data& data, const XNS::SPP& spp) {
	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  SPP   %s  SPPQueueServer", TO_CSTRING(header), TO_CSTRING(spp.toString()));

	if (spp.control.isSystem() && spp.control.isSendAck()) {
		// OK
		SPPQueue::State state;

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

			state.sst   = XNS::SPP::SST::DATA;

			// first reply packet is system, so sequence number is still 0
			state.seq   = 0;
			state.ack   = 0;
			state.alloc = 0;
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
		}


		// Send reply packet
		{
			SPP reply;
			reply.control = SPP::Control::BIT_SYSTEM;
			reply.sst = SPP::SST::DATA;
			reply.idSrc = state.localID;
			reply.idDst = state.remoteID;
			reply.seq   = state.seq;
			reply.ack   = state.ack;
			reply.alloc = state.alloc;

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


