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
// SPPPacket.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("spp-packet");

#include "../courier/Protocol.h"

#include "Server.h"
#include "SPPPacket.h"


using Courier::ExpeditedCourier;
using XNS::Server::SPPPacket;

SPPPacket* SPPPacket::clone() {
	SPPPacket* ret = new SPPPacket(*this);
	return ret;
}

void SPPPacket::run(FunctionTable functionTable) {
	logger.info("SPPPacket::run START");

	for(;;) {
		if (functionTable.stopRun()) break;
		RecvData* myData = functionTable.recv();
		if (myData == nullptr) continue;

		DEBUG_TRACE();
		logger.info("SPPPacket::run    myData  %p  %s", myData, myData->spp.block.toString());

		Data& data(myData->data);
		SPP&  spp (myData->spp);
		{
			QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
			QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
			DEBUG_TRACE();
			logger.info("%s  SPP   %s  RUN  %s", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(spp.block.toString()));
		}

		ByteBuffer level3 = spp.block.toBuffer();
		logger.info("level3  %s!", level3.toString());
		ExpeditedCourier exp;
		FROM_BYTE_BUFFER(level3, exp);

		{
			QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
			QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
			DEBUG_TRACE();
			logger.info("%s  SPP   %s  RUN  %s", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(exp.body.toString()));
		}

		Packet result;
		bool useBulk;
		myServer->getServices()->call(exp.body, result, useBulk);

		//
		// FIXME
		//

		delete myData;
	}
	logger.info("SPPPacket::run STOP");
}

