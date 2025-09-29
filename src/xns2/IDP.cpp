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

#include "../util/ByteBuffer.h"

#include "IDP.h"

namespace xns::idp {

void initialize() {
    logger.info("%s  intialize", __FUNCTION__);
}

UINT16 Checksum::NOCHECK = Checksum{0xFFFF, "****"};

UINT8 Type::RIP    = Type(1, "RIP");
UINT8 Type::ECHO   = Type(2, "ECHO");
UINT8 Type::ERROR_ = Type(3, "ERROR");
UINT8 Type::PEX    = Type(4, "PEX");
UINT8 Type::SPP    = Type(5, "SPP");
UINT8 Type::BOOT   = Type(6, "BOOT");

UINT16 Socket::RIP       = Socket(1, "RIP");
UINT16 Socket::ECHO      = Socket(2, "ECHO"); 
UINT16 Socket::ERROR_    = Socket(3, "ERROR"); 
UINT16 Socket::ENVOY     = Socket(4, "ENVOY");
UINT16 Socket::COURIER   = Socket(5, "COURIER"); 
UINT16 Socket::CHS_OLD   = Socket(7, "CHS_OLD"); 
UINT16 Socket::TIME      = Socket(8, "TIME");
			
UINT16 Socket::BOOT      = Socket(10, "BOOT"); 
UINT16 Socket::DIAG      = Socket(19, "DIAG");
			
UINT16 Socket::CHS       = Socket(20, "CHS"); 
UINT16 Socket::AUTH      = Socket(21, "AUTH"); 
UINT16 Socket::MAIL      = Socket(22, "MAIL"); 
UINT16 Socket::NETEXEC   = Socket(23, "NETEXEC"); 
UINT16 Socket::WSINFO    = Socket(24, "WSINFO"); 
UINT16 Socket::BINDING   = Socket(28, "BINDING");
			
UINT16 Socket::GERM      = Socket(35, "GERM");
UINT16 Socket::TELEDEBUG = Socket(48, "TELEDEBUG");


uint16_t computeChecksum(const ByteBuffer& bb) {
    int base  = bb.base();
    int limit = bb.limit();
    uint8_t* data = bb.data();

    uint32_t w;
    uint32_t s = 0;
    for(int i = base + 2; i < limit;) {
        w = data[i++] << 8;
        w |= data[i++];

		// add w to s
		s += w;
		// if there is overflow, increment t
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
		// shift left
		s <<= 1;
		// if there is overflow, increment t
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
    }
    return (uint16_t)s;
}

void IDP::fromByteBuffer(ByteBuffer& bb) {
        checksum.fromByteBuffer(bb);
        length.fromByteBuffer(bb);
        control.fromByteBuffer(bb);
        type.fromByteBuffer(bb);
        dstNet.fromByteBuffer(bb);
        dstHost.fromByteBuffer(bb);
        dstSocket.fromByteBuffer(bb);
        srcNet.fromByteBuffer(bb);
        srcHost.fromByteBuffer(bb);
        srcSocket.fromByteBuffer(bb);

        // FIX length using value of length field
        int newLimit = bb.position() + +length - HEADER_LENGTH;
        if (newLimit <= 0) ERROR()
        bb.limit(newLimit);

        // check checksum
        if (checksum != Checksum::NOCHECK) {
            uint16_t myChecksum = computeChecksum(bb);
            if (checksum != myChecksum) {
                // Checksum error
                logger.warn("Checksum error");
            }
        }

        block.fromByteBuffer(bb);
}

void IDP::toByteBuffer(ByteBuffer& bb) {
    int position = bb.position();
    int limit    = bb.limit();
    int length_   = limit - position;

    checksum.toByteBuffer(bb);
    length.toByteBuffer(bb);
    control.toByteBuffer(bb);
    type.toByteBuffer(bb);
    dstNet.toByteBuffer(bb);
    dstHost.toByteBuffer(bb);
    dstSocket.toByteBuffer(bb);
    srcNet.toByteBuffer(bb);
    srcHost.toByteBuffer(bb);
    srcSocket.toByteBuffer(bb);
    block.toByteBuffer(bb);

    // make odd length to even length
    if (length_ % 2) bb.write8(0);
}

}