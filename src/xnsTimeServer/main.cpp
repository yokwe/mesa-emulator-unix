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
    uint64_t                    ME;
    std::map<uint32_t, Routing> routingMap;

    Context(net::Device device, xns::config::Config config_) : config(config_), ME(device.address) {
        // build routingMap
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            routingMap[e.net] = routing;
        }
    }
    Context() : config(), ME(0) {}
};
Context context;

int main(int, char **) {
	logger.info("START");

//    xns::dumpFormatList();
    auto config = xns::config::Config::getInstance();
    logger.info("config network interface  %s", config.network.interface);
    for(const auto& e: config.host) {
        xns::Host(e.address, e.name.c_str()); // regist host name of xns::Host
        logger.info("config host  %s  %s  %s", xns::host::toHexaDecimalString(e.address), xns::host::toDecimalString(e.address), e.name);
    }
    for(const auto& e: config.net) {
        Routing routing = Routing(e.net, e.delay, e.name);
        xns::Net(e.net, e.name.c_str()); // regist net name of xns::Net
        logger.info("config net  %d  %d  %s", e.net, e.delay, e.name);
    }

	auto device = net::getDevice(config.network.interface);
	logger.info("device  %s  %s", device.name, xns::host::toHexaDecimalString(device.address));
	auto driver = net::getDriver(device);

    context = Context(device, config);

	driver->open();
	driver->discard();
    for(;;) {
        auto receiveDataList = driver->read();
        if (receiveDataList.empty()) continue;

        for(ByteBuffer receiveBB: receiveDataList) {
            // decode receiveData
            xns::ethernet::Frame receiveFrame;
            receiveFrame.fromByteBuffer(receiveBB);

            if (receiveFrame.dest != context.ME && receiveFrame.dest != xns::Host::BROADCAST) {
                logger.info("frame  %4d  %s  %s  %s  %d", receiveBB.limit(), -receiveFrame.dest, -receiveFrame.source, -receiveFrame.type, receiveFrame.block.toBuffer().remaining());
                continue;
            }

            // prepare transmitBuffer and transmitFrame
            std::array<uint8_t, xns::ethernet::Frame::MAXIMUM_LENGTH> transmitBuffer;
            transmitBuffer.fill(0);
            ByteBuffer transmitBB(transmitBuffer.size(), transmitBuffer.data());
            xns::ethernet::Frame transmitFrame;
            transmitFrame.dest = receiveFrame.source;
            transmitFrame.source = context.ME;
            transmitFrame.type   = receiveFrame.type;

            if (receiveFrame.type == xns::ethernet::Type::XNS) {
                xns::idp::IDP receiveIDP;
                auto receiveFrameData = receiveFrame.block.toBuffer();
                receiveIDP.fromByteBuffer(receiveFrameData);

                xns::idp::IDP transmitIDP;
                transmitIDP.checksum  = 0;
                transmitIDP.length    = 0;
                transmitIDP.control   = 0;
                transmitIDP.type      = receiveIDP.type;
                transmitIDP.dstNet    = receiveIDP.srcNet;
                transmitIDP.dstHost   = receiveIDP.srcHost;
                transmitIDP.dstSocket = receiveIDP.srcSocket;
                transmitIDP.srcNet    = 0;
                transmitIDP.srcHost   = context.ME;
                transmitIDP.srcSocket = receiveIDP.dstSocket;

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
        }
	}

	logger.info("STOP");
}
