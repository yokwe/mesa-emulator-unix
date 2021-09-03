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
// CHSListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-chs");

#include "../courier/Courier.h"

#include "../xnsServer/Server.h"

#include "CHSListener.h"

using ByteBuffer::Buffer;
using ByteBuffer::BLOCK;
using Network::Packet;
using XNS::Data;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::PEX;
using Courier::Services;
using Courier::Service;
using Courier::Procedure;
using Courier::ExpeditedCourier;
using Courier::Protocol3Body;
using Courier::MessageType;
using Courier::ProgramVersion;

void CHSListener::handle(const Data& data, const PEX& pex) {
	Buffer level3 = pex.block.toBuffer();
	ExpeditedCourier exp;
	FROM_BYTE_BUFFER(level3, exp);

	QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
	logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(exp.body.toString()));

	if (exp.body.type == MessageType::CALL) {
		Protocol3Body::CallBody callBody;
		exp.body.get(callBody);

		ProgramVersion programVersion((quint32)callBody.program, (quint16)callBody.version);

		Service* service = services->getService(programVersion);
		if (service == nullptr) {
			logger.warn("NO SERVICE  %s", programVersion.toString());
		} else {
			Procedure* procedure = service->getProcedure((quint16)callBody.procedure);
			if (procedure == nullptr) {
				logger.warn("NO PROCEDURE  %s  %u", service->name(), (quint16)callBody.procedure);
			} else {
				logger.info("Courier %s %s (%s)", service->name(), procedure->name(), callBody.block.toString());
				procedure->call(data, pex, callBody);
			}
		}
	} else {
		logger.error("Unexpected");
		ERROR();
	}
}

