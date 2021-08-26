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

	BPF bpf;
	{
		bpf.open();
		logger.info("bpf.fd         = %d", bpf.fd);
		logger.info("bpf.path       = %s", bpf.path);
		logger.info("bpf.bufferSize = %d", bpf.bufferSize);

		bpf.setInterface(config.network.interface);
		bpf.setPromiscuous();
		bpf.setImmediate(1);
		bpf.setHeaderComplete(0);
		bpf.setReadTimeout(10);
		bpf.setReadFilter(BPF::PROGRAM_XNS);

		logger.info("bufferSize     = %d", bpf.getBufferSize());
		logger.info("direction      = %d", bpf.getDirection());
		logger.info("headerComplete = %d", bpf.getHeaderComplete());
		logger.info("interface      = %s", bpf.getInterface());
		logger.info("nbrb           = %d", bpf.getNonBlockingReadBytes());
		logger.info("timeout        = %d", bpf.getReadTimeout());
		logger.info("buffer         = %p", bpf.buffer);
	}

	{
		bpf.discard();

		Packet level0;

		for(;;) {
			QList<ByteBuffer::Buffer> list = bpf.read();

			for(auto bb: bpf.read()) {
				int readSize = bb.limit() - bb.base();
				bb.read(bb.base(), readSize, level0.data());
				level0.setBase(0);
				level0.limit(readSize);

				QDateTime dateTime;
				{
					struct timeval* p = (struct timeval*)bb.data();
					quint64 value = (p->tv_sec * 1000) + (p->tv_usec / 1000);
					dateTime = QDateTime::fromMSecsSinceEpoch(value);
				}
				QString timeStamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");


				Ethernet ethernet;
				FROM_BYTE_BUFFER(level0, ethernet);
				Buffer level1 = ethernet.block.toBuffer();

				// check ethernet type
				if (ethernet.type != Ethernet::Type::XNS) continue;

				IDP idp;
				FROM_BYTE_BUFFER(level1, idp);
				Buffer level2 = idp.block.toBuffer();

				QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(ethernet.toString()), TO_CSTRING(idp.toString()));

				// check idp checksum
				{
					Buffer start = ethernet.block.toBuffer();
					quint16 checksum = XNS::IDP::getChecksum(start);
					if (checksum != XNS::IDP::Checksum::NOCHECK) {
						quint16 newValue = XNS::IDP::computeChecksum(start);
						if (checksum != newValue) {
							// checksum error
							logger.warn("%s  BAD CHECKSUM", header);
							continue;
						}
					}
				}

				if (idp.type == IDP::Type::RIP) {
					RIP rip;
					FROM_BYTE_BUFFER(level2, rip);
					logger.info("%s  RIP   %s", TO_CSTRING(header), TO_CSTRING(rip.toString()));
				} else if (idp.type == IDP::Type::ECHO) {
					Echo echo;
					FROM_BYTE_BUFFER(level2, echo);
					logger.info("%s  ECHO  %s", TO_CSTRING(header), TO_CSTRING(echo.toString()));
				} else if (idp.type == IDP::Type::ERROR_) {
					Error error;
					FROM_BYTE_BUFFER(level2, error);
					logger.info("%s  ERROR %s", TO_CSTRING(header), TO_CSTRING(error.toString()));
				} else if (idp.type == IDP::Type::PEX) {
					PEX pex;
					FROM_BYTE_BUFFER(level2, pex);
					Buffer level3 = pex.block.toBuffer();
					if (pex.type == PEX::Type::TIME) {
						Time time;
						FROM_BYTE_BUFFER(level3, time);
						logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(time.toString()));
					} else if (pex.type == PEX::Type::CHS) {
						ExpeditedCourier exp;
						FROM_BYTE_BUFFER(level3, exp);
						logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(exp.toString()));
					} else {
						logger.info("%s  PEX   %s  %s", TO_CSTRING(header), TO_CSTRING(pex.toString()), TO_CSTRING(pex.block.toString()));
					}
				} else if (idp.type == IDP::Type::SPP) {
					SPP spp;
					FROM_BYTE_BUFFER(level2, spp);
					logger.info("%s  SPP   %s  ???  %s", TO_CSTRING(header), TO_CSTRING(spp.toString()), TO_CSTRING(spp.block.toString()));
				} else if (idp.type == IDP::Type::BOOT) {
					Boot boot;
					FROM_BYTE_BUFFER(level2, boot);
					logger.info("%s  BOOT  %s", TO_CSTRING(header), TO_CSTRING(boot.toString()));
				} else {
					logger.info("%s  ???   %s", TO_CSTRING(header), TO_CSTRING(idp.block.toString()));
				}

			}

		}
	}

}


