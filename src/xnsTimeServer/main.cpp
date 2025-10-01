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
#include <array>
#include <cstdint>
#include <map>

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

void callInitialize() {
     xns::initialize();
}

struct Routing {
    uint32_t    net;
    uint16_t    delay;
    std::string name;

    Routing(uint32_t net_, uint16_t delay_, const std::string& name_) : net(net_), delay(delay_), name(name_) {}
    Routing() : net(0), delay(0), name("") {}
};

struct Context {
    xns::config::Config         config;
    net::Driver*                driver;
    uint64_t                    ME;
    uint32_t                    NET;
    std::map<uint32_t, Routing> routingMap;

    Context(xns::config::Config config_) : config(config_) {
        auto device = net::getDevice(config.network.interface);
        driver = net::getDriver(device);
        ME     = device.address;
        NET    = 0;
        // build routingMap
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            routingMap[e.net] = routing;
            if (routing.delay == 0) NET = e.net;
        }
    }
    Context() : config(), driver(0), ME(0), NET(0) {}
};
Context context;

void processXNS (ByteBuffer& rx, ByteBuffer& tx);
void processECHO(ByteBuffer& rx, ByteBuffer& tx);
void processPEX (ByteBuffer& rx, ByteBuffer& tx);
void processRIP (ByteBuffer& rx, ByteBuffer& tx);
void processSPP (ByteBuffer& rx, ByteBuffer& tx);

int main(int, char **) {
	logger.info("START");

//    xns::dumpFormatList();
    auto config = xns::config::Config::getInstance();
    logger.info("config network interface  %s", config.network.interface);
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
            if (receive.type == xns::ethernet::Type::XNS) {
                processXNS(rx, payload);
            }
            // if payload is empty, continue with next received data
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
                int length = tx.length();
                if (length < xns::ethernet::Frame::MINIMU_LENGTH) {
                    tx.writeZero(xns::ethernet::Frame::MINIMU_LENGTH - length);
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


void processXNS(ByteBuffer& rx, ByteBuffer& tx) {
    logger.info("XNS %4d  %s", rx.remaining(), rx.toStringFromPosition());

    xns::idp::IDP receive;
    // build receive
    {
        int base = rx.position(); // save position to fix length
        receive.fromByteBuffer(rx);
 
        uint16_t checksum;
        rx.read16(base, checksum);

        {
            auto dst = std_sprintf("%s-%s-%s", -receive.dstNet, -receive.dstHost, -receive.dstSocket);
            auto src = std_sprintf("%s-%s-%s", -receive.srcNet, -receive.srcHost, -receive.srcSocket);
            logger.info("IDP %s  %s  %s  %s  %-22s  %-22s  %d  %s",
                -receive.checksum, -receive.length, -receive.control, -receive.type,
                dst, src, rx.remaining(), rx.toStringFromPosition());
        }

        auto computedChecksum = xns::idp::computeChecksum(rx, base + 2);
//        auto checksum_A = xns::idp::computeChecksum_A(rx, position);
        logger.info("checksum  %04X  %04X", checksum, computedChecksum);

        logger.info("IDP %4d  %s", rx.remaining(), rx.toStringFromPosition());

        // check checksum
        if (receive.checksum != xns::idp::Checksum::NOCHECK && checksum != +receive.checksum) {
            // checksum error
//            logger.warn("checksum error  %04X  %04X", checksum, +receive.checksum);
//            ERROR();
        }

        // FIX length using value of length field
        if (+receive.length < xns::idp::IDP::HEADER_LENGTH) {
            logger.error("wrong length  %d", +receive.length);
            ERROR();
        }
        int newLimit = base + +receive.length;
        rx.limit(newLimit);
        logger.info("IDP %4d  %s", rx.remaining(), rx.toStringFromPosition());
    }

    EthernetPacket payload;
    if (receive.type == xns::idp::Type::ECHO) {
        processECHO(rx, payload);
    }
    if (receive.type == xns::idp::Type::PEX) {
       processPEX(rx, payload);
    }
    if (receive.type == xns::idp::Type::RIP) {
        processRIP(rx, payload);
    }
    if (receive.type == xns::idp::Type::SPP) {
        processSPP(rx, payload);
    }

    if (payload.empty()) return;

    xns::idp::IDP transmit;
    // build transmit
    {
        // make data length even
        if (payload.length() % 1) payload.writeZero(1);

        transmit.checksum  = 0;
        transmit.length    = payload.length();
        transmit.control   = 0;
        transmit.type      = receive.type;
        transmit.dstNet    = receive.srcNet;
        transmit.dstHost   = receive.srcHost;
        transmit.dstSocket = receive.srcSocket;
        transmit.srcNet    = context.NET;
        transmit.srcHost   = context.ME;
        transmit.srcSocket = receive.dstSocket;

        // write to tx
        int position = tx.position();
        transmit.toByteBuffer(tx);

        // update checksum
        uint16_t checksum = xns::idp::computeChecksum(tx, position);
        tx.write16(0, checksum);
    }
}

void processECHO(ByteBuffer& rx, ByteBuffer& tx) {
    (void)rx;
    (void)tx;
    logger.info("%s", __FUNCTION__);
}
void processPEX (ByteBuffer& rx, ByteBuffer& tx) {
    xns::pex::PEX receive(rx);
    logger.info("PEX  %s  %s  %d  %s", -receive.id, -receive.type, rx.remaining(), rx.toStringFromPosition());

    (void)rx;
    (void)tx;
    logger.info("%s", __FUNCTION__);
}
void processRIP (ByteBuffer& rx, ByteBuffer& tx) {
    (void)rx;
    (void)tx;
    logger.info("%s", __FUNCTION__);
}
void processSPP (ByteBuffer& rx, ByteBuffer& tx) {
    (void)rx;
    (void)tx;
    logger.info("%s", __FUNCTION__);
}
 

