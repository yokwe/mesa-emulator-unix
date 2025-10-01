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
 // IDP.h
 //

#pragma once

#include "Type.h"

namespace xns::idp {

void initialize();

class Checksum : public UINT16 {
    static inline const char* group = "xns::idp::Checksum";
    Checksum(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Checksum() : UINT16("%04X") {}

    UINT16 operator = (UINT16 that) {
        UINT16::operator =(that);
        return that;
    }
    uint16_t operator = (uint16_t that) {
        UINT16::operator =(that);
        return that;
    }

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT16 NOCHECK;
};

class Type : public UINT8 {
    static inline const char* group = "xns::idp::Type";
    Type(uint8_t value_, const char* name_) : UINT8(group, value_, name_) {}
public:
    Type() : UINT8("%2X") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT8 RIP;
    static UINT8 ECHO;
    static UINT8 ERROR_;
    static UINT8 PEX;
    static UINT8 SPP;
    static UINT8 BOOT;
};

class Socket : public UINT16 {
    static inline const char* group = "xns::idp::Socket";
    Socket(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Socket() : UINT16("%04X") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }

    static UINT16 RIP;
    static UINT16 ECHO;
    static UINT16 ERROR_;
    static UINT16 ENVOY;
    static UINT16 COURIER;
    static UINT16 CHS_OLD;
    static UINT16 TIME;
			
    static UINT16 BOOT;
    static UINT16 DIAG;
			
    static UINT16 CHS;
    static UINT16 AUTH;
    static UINT16 MAIL;
    static UINT16 NETEXEC;
    static UINT16 WSINFO;
    static UINT16 BINDING;
			
    static UINT16 GERM;
	static UINT16 TELEDEBUG;
};

uint16_t computeChecksum(const uint8_t* data, int start, int endPlusOne);
inline uint16_t computeChecksum(const ByteBuffer& bb, int position) { // position point to checksum field of IDP packet
    return computeChecksum(bb.data(), position + 2, bb.limit());
}

struct IDP {
    static constexpr int HEADER_LENGTH = 30;

    Checksum checksum;
    UINT16   length;
    UINT8    control;
    Type     type;

    Net      dstNet;
    Host     dstHost;
    Socket   dstSocket;

    Net      srcNet;
    Host     srcHost;
    Socket   srcSocket;

    IDP(ByteBuffer& bb);
    IDP() {}

    void fromByteBuffer(ByteBuffer& bb);
    void toByteBuffer(ByteBuffer& bb);
};

void process(IDP& receive, IDP& transmit);

}