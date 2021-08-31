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
// CourierListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-cr");

#include "../xns/SPP.h"

#include "CourierListener.h"

using ByteBuffer::Buffer;
using ByteBuffer::BLOCK;
using Network::Packet;
using XNS::Data;
using XNS::Ethernet;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::SPP;
using Courier::Services;

void CourierListener::init(Config* config_, Context* context_, Services* services_) {
	DefaultListener::init(config_, context_, services_);
	logger.info("CourierListener::init");
}
void CourierListener::start() {
	stopFuture = false;
	future = QtConcurrent::run([this](){this->run();});
}
void CourierListener::stop() {
	stopFuture = true;
	future.waitForFinished();
}
void CourierListener::run() {
	// WAIT_TIME is 1 second
	static const quint32 WAIT_TIME = 1000;

	dataList.clear();

	QMutexLocker mutexLocker(&dataListMutex);
	for(;;) {
		// loop until data is arrived
		for(;;) {
			if (stopFuture) goto exitLoop;
			bool ret = dataListCV.wait(&dataListMutex, WAIT_TIME);
			if (ret) {
				// data is arrived
				break;
			} else {
				// timeout
				continue;
			}
		}
		// process all data in dataList
		for(;;) {
			if (stopFuture) goto exitLoop;
			if (dataList.isEmpty()) break;

			MyData myData = dataList.takeLast();

			// unlock mutexLocker before lengthy process
			mutexLocker.unlock();
			{
				QString timeStamp = QDateTime::fromMSecsSinceEpoch(myData.data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
				QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(myData.data.ethernet.toString()), TO_CSTRING(myData.data.idp.toString()));
				logger.info("%s  SPP   %s  RUN", TO_CSTRING(header), TO_CSTRING(myData.spp.toString()));
//				QThread::sleep(3);
			}
			// lock mutexLocker after length process
			mutexLocker.relock();
		}
	}
exitLoop: ;
}

void CourierListener::handle(const Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::SPP) {
		SPP spp;
		FROM_BYTE_BUFFER(level2, spp);

		dataListMutex.lock();
		MyData myData(data, spp);

		QString timeStamp = QDateTime::fromMSecsSinceEpoch(myData.data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(myData.data.ethernet.toString()), TO_CSTRING(myData.data.idp.toString()));
		logger.info("%s  SPP   %s  HANDLE", TO_CSTRING(header), TO_CSTRING(myData.spp.toString()));

		dataList.append(myData);
		dataListMutex.unlock();
		dataListCV.wakeOne();

	} else if (data.idp.type == IDP::Type::ERROR_) {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}

