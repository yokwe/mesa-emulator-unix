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
 // PEX_TIME.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../xns3/Time.h"

#include "Server.h"

void processPEX_TIME(ByteBuffer& rx, ByteBuffer& tx, Context& context) {
    // build receive
    xns::time::Request receive(rx);
    logger.info("TIME >>  %s", receive.toString());

    if (receive.version != xns::time::Version::CURRENT) {
        logger.warn("Unexpected version  %s", -receive.version);
        return;
    }
    if (receive.type != xns::time::Type::REQUEST) {
        logger.warn("Unexpected type  %s", -receive.type);
        return;       
    }

    // build transmit
    xns::time::Response transmit;
    transmit.version          = +xns::time::Version::CURRENT;
    transmit.type             = +xns::time::Type::RESPONSE;
    transmit.time             = Util::getMesaTime();
    transmit.offsetDirection  = +context.config.time.offsetDirection;
    transmit.offsetHours      = context.config.time.offsetHours;
    transmit.offsetMinutes    = context.config.time.offsetMinutes;
    transmit.dstStart         = context.config.time.dstStart;
    transmit.dstEnd           = context.config.time.dstEnd;
    transmit.tolerance        = +xns::time::Tolerance::KNOWN;
    transmit.toleranceValue   = 10;
    transmit.toByteBuffer(tx);

    logger.info("TIME <<  %s", transmit.toString());
}