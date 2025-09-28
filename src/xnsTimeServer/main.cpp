/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
static const Logger logger(__FILE__);

#include "../util/net.h"
#include "../util/ByteBuffer.h"
#include "../xns2/Type.h"
#include "../xns2/Ethernet.h"
#include "../xns2/IDP.h"

void callInitialize() {
     xns::ethernet::initialize();
     xns::idp::initialize();
}

int main(int, char **) {
	logger.info("START");

//    xns::dumpFormatList();

	auto device = net::getDevice("en0");
	logger.info("device  %s", (std::string)device);
	auto driver = net::getDriver(device);

	driver->open();
	driver->discard();
    for(;;) {
        auto packets = driver->read();
        if (packets.empty()) continue;

        for(const auto& packet: packets) {
            xns::ethernet::Frame frame;
            auto packetData = packet;
            frame.fromByteBuffer(packetData);
            auto frameData = frame.block.toBuffer();

//            logger.info("frame  %4d  %s  %s  %s  %d", frameData.limit(), -frame.dest, -frame.source, -frame.type, idpData.remaining());

            if (frame.type == xns::ethernet::Type::XNS) {
                xns::idp::IDP idp;
                idp.fromByteBuffer(frameData);

                auto idpData = idp.block.toBuffer();

                auto dst = std_sprintf("%s-%s-%s", -idp.dstNet, -idp.dstHost, -idp.dstSocket);
                auto src = std_sprintf("%s-%s-%s", -idp.srcNet, -idp.srcHost, -idp.srcSocket);

                logger.info("%s  %s  %s  %s  %-22s  %-22s  %d",
                    -idp.checksum, -idp.length, -idp.control, -idp.type,
                    dst, src, idpData.remaining());
            }
        }
	}

	logger.info("STOP");
}
