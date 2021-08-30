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
// TimeListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-time");

#include "../xns/Time.h"

#include "TimeListener.h"

using ByteBuffer::Buffer;
using ByteBuffer::BLOCK;
using Network::Packet;;
using XNS::Data;
using XNS::Config;
using XNS::Context;
using XNS::IDP;
using XNS::PEX;
using XNS::Time;
using Courier::Services;

void TimeListener::init(Config* config_, Context* context_, Services* services_) {
	(void)config_;
	(void)context_;
	(void)services_;
	logger.info("TimeListener::init");
}
void TimeListener::handle(const Data& data) {
	Buffer level2 = data.idp.block.toBuffer();
	if (data.idp.type == IDP::Type::PEX) {
		PEX pex;
		FROM_BYTE_BUFFER(level2, pex);

		if (pex.type == PEX::Type::TIME) {
			Buffer level3 = pex.block.toBuffer();
			Time time;
			FROM_BYTE_BUFFER(level3, time);

			QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
			QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
			logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(time.toString()));

			if (time.type == Time::Type::REQUEST) {
				Time::Response response;
				response.time            = QDateTime::currentSecsSinceEpoch();
				response.offsetDirection = data.config.time.offsetDirection;
				response.offsetHours     = data.config.time.offsetHours;
				response.offsetMinutes   = data.config.time.offsetMinutes;
				response.tolerance       = Time::Tolerance::MILLI;
				response.toleranceValue  = 10;

				Time replyTime;
				replyTime.version = Time::Version::CURRENT;
				replyTime.type    = Time::Type::RESPONSE;
				replyTime.set(response);

				Packet level3;
				TO_BYTE_BUFFER(level3, replyTime);
				BLOCK block3(level3);

				// set block3 to replyPEX.block
				PEX replyPEX;
				replyPEX.id    = pex.id;
				replyPEX.type  = PEX::Type::TIME;
				replyPEX.block = block3;

				DefaultListener::transmit(data, replyPEX);
			} else {
				logger.error("Unexpected");
				logger.error("  time %s", time.toString());
				ERROR();
			}

		} else {
			logger.error("Unexpected");
			logger.error("    %s", data.idp.toString());
			logger.error("        %s", data.idp.block.toString());
			ERROR();
		}

	} else if (data.idp.type == IDP::Type::ERROR_) {
		logger.error("Unexpected");
		logger.error("    %s", data.idp.toString());
		logger.error("        %s", data.idp.block.toString());
		ERROR();
	}
}

