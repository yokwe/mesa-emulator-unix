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
#include "../util/ThreadControl.h"
#include "../util/ThreadQueue.h"

#include "../xns3/Type.h"
#include "../xns3/Ethernet.h"
#include "../xns3/Config.h"

#include "Server.h"

struct ThreadTransmit : thread_queue::ThreadQueueProcessor<net::Packet> {
    net::Driver& driver;

    ThreadTransmit(net::Driver& driver_) : driver(driver_) {}

    void process(const net::Packet& data) {
        driver.write(data);
    }
};

struct ThreadReceive : thread_queue::ThreadQueueProducer<net::Packet> {
    net::Driver& driver;

    ThreadReceive(net::Driver& driver_) : driver(driver_) {}

    bool produce(net::Packet& packet, std::chrono::milliseconds timeout) {
        packet.clear();
        int opErrno;
        if (driver.select(timeout, opErrno)) {
            int length = driver.receive(packet.data(), packet.capacity(), opErrno);
            packet.limit(length);
            return true;
        }
        return false;
    }
};


Context context;

int main(int, char **) {
	logger.info("START");

    setSignalHandler(SIGINT);
	setSignalHandler(SIGTERM);
	setSignalHandler(SIGHUP);
	setSignalHandler(SIGSEGV);

    auto config = xns::config::Config::getInstance();
    logger.info("config network interface  %s", config.server.interface);
    // register constant of host and net from config
    {
        {
            auto address = config.server.address;
            auto name = config.server.name;
            xns::Host::registerName(address, name);
            logger.info("config host  %s  %s  %s", net::toHexaDecimalString(address), net::toDecimalString(address), name);
        }
        for(const auto& e: config.host) {
            xns::Host::registerName(e.address, e.name);
            logger.info("config host  %s  %s  %s", net::toHexaDecimalString(e.address), net::toDecimalString(e.address), e.name);
        }
        for(const auto& e: config.net) {
            Routing routing = Routing(e.net, e.delay, e.name);
            xns::Net::registerName(e.net, e.name);
            logger.info("config net  %d  %d  %s", e.net, e.delay, e.name);
        }
    }

    context = Context(config);
	logger.info("device  %s  %s", context.driver->device.name, net::toHexaDecimalString(context.driver->device.address));
	logger.info("ME      %s", net::toHexaDecimalString(context.ME));
	logger.info("NET     %d", context.NET);

    auto& driver = *context.driver;
	driver.open();

    ThreadReceive  threadReceive(driver);
    ThreadTransmit threadTransmit(driver);

    std::function<void()> f1 = std::bind(&ThreadReceive::run, &threadReceive);
    std::function<void()> f2 = std::bind(&ThreadTransmit::run, &threadTransmit);

	ThreadControl t1("threadReceive",  f1);
	ThreadControl t2("threadTransmit", f2);

    driver.discard();
    t1.start();
    t2.start();

    for(;;) {
        net::Packet rx;
        threadReceive.pop(rx);
        if (rx.empty()) continue;

        // build receive
        xns::ethernet::Frame receive(rx);
        if (receive.dest != context.ME && receive.dest != xns::Host::BROADCAST) {
            // not my address or not broadcast
            // logger.info("frame  %s  %d", receive.toString(), rx.remaining());
            continue;
        }
        logger.info("ETH  >>  %s  %d", receive.toString(), rx.remaining());


        net::Packet payload;
        if (receive.type == xns::ethernet::Type::XNS) processIDP(rx, payload, context);
        // if payload is empty, continue with next received data
        payload.flip();
        // logger.info("payload  length  %d", payload.length());
        if (payload.empty()) continue;

        xns::ethernet::Frame transmit;
        // build transmit
        {
            transmit.dest   = receive.source;
            transmit.source = context.ME;
            transmit.type   = receive.type;
        }
        logger.info("ETH  <<  %s  %d", transmit.toString(), payload.remaining());

        net::Packet tx;
        // build tx
        {
            transmit.toByteBuffer(tx);
            tx.write(payload.length(), payload.data());
            // add padding if it is smaller than MINIMUM_LENGTH
            int length = tx.length();
            if (length < xns::ethernet::Frame::MINIMUM_LENGTH) {
                tx.writeZero(xns::ethernet::Frame::MINIMUM_LENGTH - length);
            }
            // logger.info("TX  length  %d", tx.length());
        }
        threadTransmit.push(tx);
	}

    threadReceive.stop();
    threadTransmit.stop();
    t1.join();
    t2.join();

	logger.info("STOP");
}
