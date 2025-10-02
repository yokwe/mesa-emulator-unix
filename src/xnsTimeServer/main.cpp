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

 
 //
 // main.cpp
 //


#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/net.h"
#include "../util/ByteBuffer.h"
#include "../util/EthernetPacket.h"

#include "../xns2/Type.h"
#include "../xns2/Ethernet.h"
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
            logger.info("config host  %s  %s  %s", net::toHexaDecimalString(e.address), net::toDecimalString(e.address), e.name);
        }
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            (void)xns::Net(e.net, e.name.c_str()); // regist net name of xns::Net
            logger.info("config net  %d  %d  %s", e.net, e.delay, e.name);
        }
    }

    context = Context(config);
	logger.info("device  %s  %s", context.driver->device.name, net::toHexaDecimalString(context.driver->device.address));
	logger.info("ME      %s", net::toHexaDecimalString(context.ME));
	logger.info("NET     %d", context.NET);

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
                tx.flip();
                logger.info("TX  length  %d", tx.length());
            }
            driver.write(tx);
        }
	}

	logger.info("STOP");
}
