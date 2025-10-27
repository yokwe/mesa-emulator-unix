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


#undef  DECL_CLASS_CONSTANT
#define DECL_CLASS_CONSTANT(type, name, value) constantMap.map[type :: name ] = #name;

void Checksum::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Checksum, NOCHECK,  0xFFFF)
}
void Type::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Type, RIP,    1)
    DECL_CLASS_CONSTANT(Type, ECHO,   2)
    DECL_CLASS_CONSTANT(Type, ERROR_, 3)
    DECL_CLASS_CONSTANT(Type, PEX,    4)
    DECL_CLASS_CONSTANT(Type, SPP,    5)
    DECL_CLASS_CONSTANT(Type, BOOT,   6)
}
void Socket::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Socket, RIP,       1)
    DECL_CLASS_CONSTANT(Socket, ECHO,      2)
    DECL_CLASS_CONSTANT(Socket, ERROR_,    3)
    DECL_CLASS_CONSTANT(Socket, ENVOY,     4)
    DECL_CLASS_CONSTANT(Socket, COURIER,   5)
    DECL_CLASS_CONSTANT(Socket, CHS_OLD,   7)
    DECL_CLASS_CONSTANT(Socket, TIME,      8)
                
    DECL_CLASS_CONSTANT(Socket, BOOT,      10)
    DECL_CLASS_CONSTANT(Socket, DIAG,      19)
                
    DECL_CLASS_CONSTANT(Socket, CHS,       20)
    DECL_CLASS_CONSTANT(Socket, AUTH,      21)
    DECL_CLASS_CONSTANT(Socket, MAIL,      22)
    DECL_CLASS_CONSTANT(Socket, NETEXEC,   23)
    DECL_CLASS_CONSTANT(Socket, WSINFO,    24)
    DECL_CLASS_CONSTANT(Socket, BINDING,   28)
                
    DECL_CLASS_CONSTANT(Socket, GERM,      35)
    DECL_CLASS_CONSTANT(Socket, TELEDEBUG, 48)
}

uint16_t computeChecksum(const uint8_t* data, int start, int endPlusOne) {
    uint32_t s = 0;
    for(int i = start; i < endPlusOne;) {
        uint32_t w = (data[i++] & 0xFFFFU) << 8;
        w |= data[i++];

		// add w to s
		s += w;
		// if there is overflow, increment s
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
		// shift left
		s <<= 1;
		// if there is overflow, increment s
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
    }

    // From page 21 of INTERNET TRANSPORT PROTOCOLS
    // If the result of the checksumming operation is the one's complement value "minus zero" (177777 octal),
    // it should be converted to "plus zero" (0 octal).
    uint16_t result = (uint16_t)s;
    return (result == 0xFFFFU) ? 0 : result;
}

}