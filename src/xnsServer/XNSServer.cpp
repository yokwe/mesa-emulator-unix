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
static const Logger logger = Logger::getLogger("xns-server");

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

#include "XNSServer.h"


void XNS::Server::init(const QString& path) {
	config = XNS::loadConfig(path);

	{
		QList<Network::Device> list = Network::getDeviceList();
		for(auto e: list) {
			if (e.name == config.interface) {
				device = e;
			}
		}
		if (device.isNull()) {
			logger.error("Unexpected interface");
			logger.error("  interface %s", config.interface);
			for(auto e: list) {
				logger.error("  valid interface \"%s\"", e.name);
			}
			ERROR();
		}
		logger.info("device.name    = %s", device.name);
		logger.info("device.address = %s", XNS::IDP::Host::toHexaDecimalString(device.address, ":"));
	}

	bpf.open();
	logger.info("bpf.fd         = %d", bpf.fd);
	logger.info("bpf.path       = %s", bpf.path);
	logger.info("bpf.bufferSize = %d", bpf.bufferSize);

	bpf.setInterface(config.interface);
	bpf.setPromiscious();
	bpf.setImmediate(1);
	bpf.setHeaderComplete(0);
	bpf.setReadTimeout(0);
	bpf.setReadFilter(BPF::PROGRAM_XNS);

	logger.info("bufferSize     = %d", bpf.getBufferSize());
	logger.info("direction      = %d", bpf.getDirection());
	logger.info("headerComplete = %d", bpf.getHeaderComplete());
	logger.info("interface      = %s", bpf.getInterface());
	logger.info("nbrb           = %d", bpf.getNonBlockingReadBytes());
	logger.info("timeout        = %d", bpf.getReadTimeout());
	logger.info("buffer         = %p", bpf.buffer);
}

void XNS::Server::start() {
	// FIXME
	// Start thread
}
void XNS::Server::stop() {
	requestStop = true;
}

void XNS::Server::run() {
	logger.info("run start");
	bpf.flush();

	int opErrno;
	for(;;) {
		for(;;) {
			if (requestStop) goto exitLoop;
			int ret = bpf.select(1, opErrno);
			if (ret < 0) {
				LOG_ERRNO(opErrno);
				ERROR();
			}
			if (0 < ret) break;
		}
		if (requestStop) goto exitLoop;
		const QList<ByteBuffer::Buffer>& list = bpf.read();
		logger.info("list %d", list.size());

		for(ByteBuffer::Buffer e: list) {
			if (requestStop) goto exitLoop;

			Network::Packet packet(e);

			XNS::Ethernet ethernet;
			FROM_BYTE_BUFFER(e, ethernet);
			ByteBuffer::Buffer level1 = ethernet.block.toBuffer();

			if (ethernet.type != XNS::Ethernet::Type::XNS) continue;

			XNS::IDP idp;
			FROM_BYTE_BUFFER(level1, idp);

			logger.info("%s", ethernet.toString());
			logger.info("    %s", idp.toString());

			quint16 socket = (quint16)idp.dstSocket;
			if (handlerMap.contains(socket)) {
				Handler* handler = handlerMap[socket];
				try {
					handler->process(ethernet, idp);
				} catch(const XNS::Error& error) {
					logger.info("Catch Error");
					logger.info("  error = %s", error.toString());
					// FIXME
				}
			} else {
				logger.warn("no handler for socket %s", idp.dstSocket.toString());
			}
		}
	}

exitLoop:
	logger.info("run stop");
	requestStop = false;
}
