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


 

#include <cstdint>
#include <map>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/net.h"
#include "../util/ByteBuffer.h"
#include "../util/EthernetPacket.h"

#include "../xns2/Type.h"
#include "../xns2/Ethernet.h"
#include "../xns2/IDP.h"
#include "../xns2/PEX.h"
#include "../xns2/RIP.h"
#include "../xns2/SPP.h"
#include "../xns2/Time.h"
#include "../xns2/Config.h"

#include "Server.h"

void callInitialize() {
     xns::initialize();
}

Context context;

int main(int, char **) {
	logger.info("START");

//    xns::dumpFormatList();
    auto config = xns::config::Config::getInstance();
    logger.info("config network interface  %s", config.server.interface);
    // register constant of host and net from config
    {
        for(const auto& e: config.host) {
            (void)xns::Host(e.address, e.name.c_str()); // regist host name of xns::Host
            logger.info("config host  %s  %s  %s", xns::host::toHexaDecimalString(e.address), xns::host::toDecimalString(e.address), e.name);
        }
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            (void)xns::Net(e.net, e.name.c_str()); // regist net name of xns::Net
            logger.info("config net  %d  %d  %s", e.net, e.delay, e.name);
        }
    }

    context = Context(config);
	logger.info("device  %s  %s", context.driver->device.name, xns::host::toHexaDecimalString(context.driver->device.address));

    auto& driver = *context.driver;
	driver.open();
	driver.discard();
    for(;;) {
        auto receiveDataList = driver.read();
        if (receiveDataList.empty()) continue;

        for(ByteBuffer rx: receiveDataList) {
//            logger.info("RX  %4d  %s", rx.length(), rx.toStringFromBase());
            
            // build receive
            xns::ethernet::Frame receive(rx);
//            logger.info("frame  %4d  %s  %s  %s  %d", rx.length(), -receive.dest, -receive.source, -receive.type, rx.remaining());

            if (receive.dest != context.ME && receive.dest != xns::Host::BROADCAST) {
                // not my address or not broadcast
                logger.info("frame  %4d  %s  %s  %s  %d", rx.limit(), -receive.dest, -receive.source, -receive.type, rx.remaining());
                continue;
            }

            EthernetPacket payload;
            if (receive.type == xns::ethernet::Type::XNS) processIDP(rx, payload, context);
            // if payload is empty, continue with next received data
            payload.flip();
            logger.info("payload  length  %d", payload.length());
            if (payload.empty()) continue;
            continue;

            xns::ethernet::Frame transmit;
            // build transmit
            {
                transmit.dest   = receive.source;
                transmit.source = context.ME;
                transmit.type   = receive.type;
            }

            EthernetPacket tx;
            // build tx
            {
                transmit.toByteBuffer(tx);
                tx.write(payload.length(), payload.data());
                // add padding if it is smaller than MINIMUM_LENGTH
                int length = tx.length();
                if (length < xns::ethernet::Frame::MINIMUM_LENGTH) {
                    tx.writeZero(xns::ethernet::Frame::MINIMUM_LENGTH - length);
                }
            }
            driver.write(tx);
        }

/*
        if (receiveFrame.type == xns::ethernet::Type::XNS) {
            xns::idp::IDP receiveIDP;
            auto receiveFrameData = receiveFrame.block.toBuffer();
            receiveIDP.fromByteBuffer(receiveFrameData);
            auto idpData = receiveIDP.block.toBuffer();

            auto dst = std_sprintf("%s-%s-%s", -receiveIDP.dstNet, -receiveIDP.dstHost, -receiveIDP.dstSocket);
            auto src = std_sprintf("%s-%s-%s", -receiveIDP.srcNet, -receiveIDP.srcHost, -receiveIDP.srcSocket);

            logger.info("%s  %s  %s  %s  %-22s  %-22s  %d",
                -receiveIDP.checksum, -receiveIDP.length, -receiveIDP.control, -receiveIDP.type,
                dst, src, idpData.remaining());
            
            if (receiveIDP.type == xns::idp::Type::PEX) {
                xns::pex::PEX pex;
                pex.fromByteBuffer(idpData);

                auto pexData = pex.block.toBuffer();

                logger.info("    PEX  %s  %s  %s", -pex.id, -pex.type, pex.block.toString());

                if (pex.type == xns::pex::Type::TIME) {
                    xns::time::Request request;
                    request.fromByteBuffer(pexData);

                    logger.info("        TIME  %s  %s", -request.version, -request.type);
                }
                continue;
            }
            if (receiveIDP.type == xns::idp::Type::RIP) {
                xns::rip::RIP rip;
                rip.fromByteBuffer(idpData);
                std::string string;
                for(const auto& entry: rip.table) {
                    string += std_sprintf(" {%s %s}", -entry.net, -entry.delay);
                }
                logger.info("    RIP  %s  %s", -rip.type, string.substr(1));
                //
                continue;
            }
            if (receiveIDP.type == xns::idp::Type::SPP) {
                continue;
            }
            if (receiveIDP.type == xns::idp::Type::ERROR_) {
                continue;
            }
            if (receiveIDP.type == xns::idp::Type::ECHO) {
                continue;
            }
        }
*/
	}

	logger.info("STOP");
}
