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
 // IDP.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../xns2/IDP.h"

#include "../util/EthernetPacket.h"

#include "Server.h"

void processIDP(ByteBuffer& rx, ByteBuffer& tx, Context& context) {
//    logger.info("IDP %4d  %s", rx.remaining(), rx.toStringFromPosition());

    xns::idp::IDP receive;
    // build receive
    {
        int base = rx.position(); // save position to fix length
        receive.fromByteBuffer(rx);

//        logger.info("IDP %4d  %s", rx.remaining(), rx.toStringFromPosition());

        // FIX length using value of length field
        if (+receive.length < xns::idp::IDP::HEADER_LENGTH) {
            logger.error("wrong length  %d", +receive.length);
            ERROR();
        }
        int newLimit = base + +receive.length;
        rx.limit(newLimit);
//        logger.info("IDP %4d  %s", rx.remaining(), rx.toStringFromPosition());
 
        {
            auto dst = std_sprintf("%s-%s-%s", -receive.dstNet, -receive.dstHost, -receive.dstSocket);
            auto src = std_sprintf("%s-%s-%s", -receive.srcNet, -receive.srcHost, -receive.srcSocket);
            logger.info("IDP %s  %s  %s  %s  %-22s  %-22s  %d  %s",
                -receive.checksum, -receive.length, -receive.control, -receive.type,
                dst, src, rx.remaining(), rx.toStringFromPosition());
        }

        // check checksum
        if (receive.checksum != xns::idp::Checksum::NOCHECK) {
            uint16_t checksum;
            rx.read16(base, checksum);

            auto computedChecksum = xns::idp::computeChecksum(rx, base);
            if (checksum != computedChecksum) {
                // checksum error
                logger.warn("checksum  %04X  %04X", checksum, computedChecksum);
                // TODO return error packet
                ERROR();
            }
        }
    }

    EthernetPacket payload;
    if (receive.type == xns::idp::Type::ECHO) {
        processECHO(rx, payload, context);
    }
    if (receive.type == xns::idp::Type::PEX) {
       processPEX(rx, payload, context);
    }
    if (receive.type == xns::idp::Type::RIP) {
        processRIP(rx, payload, context);
    }
    if (receive.type == xns::idp::Type::SPP) {
        processSPP(rx, payload, context);
    }
    payload.flip();
    logger.info("payload  length  %d", payload.length());
    if (payload.empty()) return;

    xns::idp::IDP transmit;
    // build transmit
    {
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
    }

    // write to tx
    {
        int base = tx.position();
        transmit.toByteBuffer(tx);
        tx.write(payload.limit(), payload.data());
        // make packet length even
        if (tx.limit() % 1) tx.writeZero(1);

        // update checksum
        uint16_t checksum = xns::idp::computeChecksum(tx, base);
        tx.write16(base, checksum);
    }
}
