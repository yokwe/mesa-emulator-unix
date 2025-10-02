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
 // RIP.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../xns2/RIP.h"

#include "Server.h"

using namespace xns::rip;

void processRIP(ByteBuffer& rx, ByteBuffer& tx, Context& context) {
    // build receive
    RIP receive;
    receive.fromByteBuffer(rx);
    logger.info("RiP  %s  %d", -receive.type, receive.table.size());
    for(auto& e: receive.table) {
        logger.info("     %s  %s", -e.net, -e.delay);
    }

    if (receive.type != Type::REQUEST) {
        logger.warn("Unexpected type  %s", -receive.type);
        return;       
    }

    // build transmit
    RIP transmit;
    transmit.type = Type::RESPONSE;

    for(auto e: context.config.net) {
        NetDelay netDelay(e.net, e.delay);
        transmit.table.push_back(netDelay);
    }
    logger.info("RiP  %s  %d", -transmit.type, transmit.table.size());
    for(auto& e: transmit.table) {
        logger.info("     !%s!  !%s", -e.net, -e.delay);
    }

    // write to tx
    transmit.toByteBuffer(tx);
}