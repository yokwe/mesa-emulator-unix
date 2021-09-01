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

#include "../xns/SPP.h"

#include "SPPQueue.h"

using ByteBuffer::Buffer;
using Network::Packet;
using XNS::Data;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::SPP;
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

void SPPQueue::start() {
	stopFuture = false;

	future = QtConcurrent::run([this](){this->run(functionTable);});
}
void SPPQueue::stop() {
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
