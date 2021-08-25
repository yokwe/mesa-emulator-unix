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
// main.pp
//


#include "../util/Util.h"
static const Logger logger = Logger::getLogger("main");

#include "../xns/XNS.h"
#include "../xns/Config.h"
#include "../xns/Server.h"
#include "../xns/RIP.h"
#include "../xns/Echo.h"
#include "../xns/Error.h"
#include "../xns/PEX.h"
#include "../xns/SPP.h"
#include "../xns/Boot.h"
#include "../xns/Time.h"
#include "../xns/Courier.h"

void xnsDump();

int main(int, char**) {
	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	logger.info("START");
	xnsDump();
	logger.info("STOP");

	return 0;
}

void xnsDump() {
	using ByteBuffer::Buffer;
	using Network::Packet;
	using XNS::Config;
	using XNS::Ethernet;
	using XNS::IDP;
	using XNS::RIP;
	using XNS::Echo;
	using XNS::Error;
	using XNS::PEX;
	using XNS::SPP;
	using XNS::Boot;
	using XNS::Time;
	using XNS::Server::Context;
	using XNS::Courier::ExpeditedCourier;


	Config config = XNS::loadConfig("tmp/run/xns-config.json");
	Context context = XNS::Server::Context(config);

	{

		context.driver->discard();

		int ret;
		int opErrno;
		Packet packet;

		for(;;) {
			// loop until data arrive
			for(;;) {
				ret = context.driver->select(1, opErrno);
				if (ret < 0) {
					LOG_ERRNO(opErrno);
					ERROR();
				}
				if (0 < ret) break;
			}

			// receive one data
			{
				ret = context.driver->receive(packet.data(), packet.limit(), opErrno);
				if (ret < 0) {
					logger.warn("Unexpected");
					LOG_ERRNO(opErrno);
					continue;
				}
				packet.position(0);
				packet.limit(ret);
			}

			Ethernet ethernet;
			FROM_BYTE_BUFFER(packet, ethernet);
			Buffer level1 = ethernet.block.toBuffer();

			// check ethernet type
			if (ethernet.type != Ethernet::Type::XNS) continue;

			// set start for checksum
			Buffer start = level1.newBase();

			IDP idp;
			FROM_BYTE_BUFFER(level1, idp);
			Buffer level2 = idp.block.toBuffer();

			logger.info("%s", ethernet.toString());
			logger.info("    %s", idp.toString());

			// check idp checksum
			{
				quint16 checksum = XNS::IDP::getChecksum(start);
				if (checksum != XNS::IDP::Checksum::NOCHECK) {
					quint16 newValue = XNS::IDP::computeChecksum(start);
					if (checksum != newValue) {
						// checksum error
						logger.warn("Checksum error");
						continue;
					}
				}
			}

			if (idp.type == IDP::Type::RIP) {
				RIP rip;
				FROM_BYTE_BUFFER(level2, idp);
				// FIXME
			} else if (idp.type == IDP::Type::ECHO) {
				Echo echo;
				FROM_BYTE_BUFFER(level2, echo);
				// FIXME
			} else if (idp.type == IDP::Type::ERROR_) {
				Error error;
				FROM_BYTE_BUFFER(level2, error);
				// FIXME
			} else if (idp.type == IDP::Type::PEX) {
				PEX pex;
				FROM_BYTE_BUFFER(level2, pex);
				Buffer level3 = pex.block.toBuffer();
				if (pex.type == PEX::Type::TIME) {
					Time time;
					FROM_BYTE_BUFFER(level3, time);
					// FIXME
				} else if (pex.type == PEX::Type::CHS) {
					ExpeditedCourier exp;
					FROM_BYTE_BUFFER(level3, exp);
					// FIXME
				} else {
					// FIXME
				}
				// FIXME
			} else if (idp.type == IDP::Type::SPP) {
				SPP spp;
				FROM_BYTE_BUFFER(level2, spp);
				Buffer level3 = spp.block.toBuffer();
				// FIXME
			} else if (idp.type == IDP::Type::BOOT) {
				Boot boot;
				FROM_BYTE_BUFFER(level2, boot);
				// FIXME
			} else {
				//
			}

		}
exitLoop:
		/* empty statement for label */ ;

	}

}


