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

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("main");

#include "../util/Network.h"
#include "../util/BPF.h"

#include "../xns/Config.h"
#include "../xns/XNS.h"
#include "../xns/Courier.h"

#include "../xns/RIP.h"
#include "../xns/Error.h"
#include "../xns/Echo.h"
#include "../xns/SPP.h"
#include "../xns/PEX.h"

#include "../xns/Time.h"

#include "../util/JSONUtil.h"
#include "../util/ByteBuffer.h"

void testNetwork() {
	{
		QList<Network::Device> list = Network::getDeviceList();
		for(auto e: list) {
			logger.info("device %s", e.toString());
//			logger.info("       %s", XNS::IDP::Host::toOctalString(e.address));
//			logger.info("       %s", XNS::IDP::Host::toDecimalString(e.address));
//			logger.info("       %s", XNS::IDP::Host::toHexaDecimalString(e.address));
			logger.info("       %s", XNS::IDP::Host::toHexaDecimalString(e.address, ":"));
		}
	}

	XNS::Config config = XNS::loadConfig("tmp/run/xns-config.json");

	{
		BPF bpf;

		bpf.open();
		logger.info("bpf.fd         = %d", bpf.fd);
		logger.info("bpf.path       = %s", bpf.path);
		logger.info("bpf.bufferSize = %d", bpf.bufferSize);

		bpf.setInterface(config.interface);
		bpf.setPromiscious();
		bpf.setImmediate(0);
		bpf.setHeaderComplete(0);
		bpf.setReadTimeout(1);
		bpf.setReadFilter(BPF::PROGRAM_XNS);

		logger.info("bufferSize     = %d", bpf.getBufferSize());
		logger.info("direction      = %d", bpf.getDirection());
		logger.info("headerComplete = %d", bpf.getHeaderComplete());
		logger.info("interface      = %s", bpf.getInterface());
		logger.info("nbrb           = %d", bpf.getNonBlockingReadBytes());
		logger.info("timeout        = %d", bpf.getReadTimeout());
		logger.info("buffer         = %p", bpf.buffer);

		logger.info("====================");
		for(;;) {
			const QList<ByteBuffer::Buffer>& list = bpf.read();
			for(ByteBuffer::Buffer e: list) {
				Network::Packet packet(e);
//				logger.info("%3d list           = %s", i, packet.toString(60));

				XNS::Ethernet ethernet;
				FROM_BYTE_BUFFER(e, ethernet);
				ByteBuffer::Buffer level1 = ethernet.block.toBuffer();

				// Check enternet.type
				// Even if BPF filter is set, first packet can be not XNS packet
				if (ethernet.type == XNS::Ethernet::Type::XNS) {
					XNS::IDP idp;
					FROM_BYTE_BUFFER(level1, idp);
					ByteBuffer::Buffer level2 = idp.block.toBuffer();

					logger.info("%s", ethernet.toString());
					logger.info("    %s", idp.toString());

					if (idp.type == XNS::IDP::Type::RIP) {
						XNS::RIP rip;
						FROM_BYTE_BUFFER(level2, rip);

						logger.info("        RIP  %s", rip.toString());
					} else if (idp.type == XNS::IDP::Type::ERROR_) {
						XNS::Error error;
						FROM_BYTE_BUFFER(level2, error);
						ByteBuffer::Buffer level3 = error.block.toBuffer();

						logger.info("        ERROR  %s", error.toString());
						logger.info("            %s", level3.toString());
					} else if (idp.type == XNS::IDP::Type::ECHO) {
						XNS::Echo echo;
						FROM_BYTE_BUFFER(level2, echo);

						logger.info("        ECHO   %s", echo.toString());
					} else if (idp.type == XNS::IDP::Type::PEX) {
						XNS::PEX pex;
						FROM_BYTE_BUFFER(level2, pex);
						ByteBuffer::Buffer level3 = pex.block.toBuffer();

						logger.info("        PEX  %s", pex.toString());
						if (pex.type == XNS::PEX::Type::TIME) {
							XNS::Time time;
							FROM_BYTE_BUFFER(level3, time);

							logger.info("            TIME  %s", time.toString());
						} else if (pex.type == XNS::PEX::Type::CHS) {
							XNS::Courier::ExpeditedCourier expeditedCourier;
							FROM_BYTE_BUFFER(level3, expeditedCourier);

							logger.info("            CHS   %s", expeditedCourier.toString());
						} else {
							logger.info("            %s", level3.toString());
						}
					} else if (idp.type == XNS::IDP::Type::SPP) {
						XNS::SPP spp;
						FROM_BYTE_BUFFER(level2, spp);
						ByteBuffer::Buffer level3 = spp.block.toBuffer();

						logger.info("        SPP  %s", spp.toString());
						logger.info("            %s", level3.toString());
					} else {
						logger.info("        ????  %s", level2.toString());
					}
				}
			}
		}

		bpf.close();
	}
}


int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	DEBUG_TRACE();

	testNetwork();

	logger.info("STOP");
	return 0;
}


