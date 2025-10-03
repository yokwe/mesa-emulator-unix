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
 // PEX.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../xns3/PEX.h"

#include "../util/EthernetPacket.h"

#include "Server.h"

void processPEX(ByteBuffer& rx, ByteBuffer& tx, Context& context) {
    (void)tx; (void)context;
//    logger.info("remaining  %d", rx.remaining());

    // build receive
    xns::pex::PEX receive(rx);
    logger.info("PEX  >>  %s  (%d) %s", receive.toString(), rx.remaining(), rx.toStringFromPosition());

    EthernetPacket payload;
    if (receive.type == xns::pex::Type::UNSPEC) {
        //
    }
    if (receive.type == xns::pex::Type::TIME) {
        processPEX_TIME(rx, payload, context);
    }
    if (receive.type == xns::pex::Type::CHS) {

    }
    if (receive.type == xns::pex::Type::TELEDEBUG) {

    }
    payload.flip();
    // logger.info("payload  length  %d", payload.length());
    if (payload.empty()) {
        logger.info("REJECT");
        return;
    }
    xns::pex::PEX transmit;
    {
        transmit.id  = receive.id;
        transmit.type = receive.type;
    }
    transmit.toByteBuffer(tx);
    tx.write(payload.limit(), payload.data());
    logger.info("PEX  <<  %s  (%d) %s", transmit.toString(), payload.limit(), payload.toString());
}
