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
 // ECHO.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../xns2/Echo.h"

#include "../util/EthernetPacket.h"

#include "Server.h"

void processECHO(ByteBuffer& rx, ByteBuffer& tx, Context& context) {
    (void)context;
    // build receive
    xns::echo::Echo receive(rx);
    logger.info("ECHO %s  %d", -receive.type, rx.remaining());

    if (receive.type != xns::echo::Type::REQUEST) {
        logger.warn("Unexpected type  %s", -receive.type);
        return;       
    }

    // build payload
    EthernetPacket payload;
    {
        // copy remaaining content of rx to payload
        uint8_t data;
        while(rx.hasRemaining()) {
            rx.read8(data);
            payload.write8(data);
        }
    }

    // build transmit
    xns::echo::Echo transmit(+xns::echo::Type::RESPONSE);

    // write to tx
    transmit.toByteBuffer(tx);
    tx.write(payload.limit(), payload.data());
}