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
 // Server.h
 //

#pragma once

#include <string>
#include <map>

#include "../xns2/Config.h"
#include "../util/net.h"


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
        auto device = net::getDevice(config.server.interface);
        driver = net::getDriver(device);
        ME     = config.server.address;
        NET    = config.server.net;
        // build routingMap
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            routingMap[e.net] = routing;
        }
    }
    Context() : config(), driver(0), ME(0), NET(0) {}
};

void processIDP      (ByteBuffer& rx, ByteBuffer& tx, Context& context);
void processECHO     (ByteBuffer& rx, ByteBuffer& tx, Context& context);
void processPEX      (ByteBuffer& rx, ByteBuffer& tx, Context& context);
void processPEX_TIME (ByteBuffer& rx, ByteBuffer& tx, Context& context);
void processRIP      (ByteBuffer& rx, ByteBuffer& tx, Context& context);
void processSPP      (ByteBuffer& rx, ByteBuffer& tx, Context& context);


