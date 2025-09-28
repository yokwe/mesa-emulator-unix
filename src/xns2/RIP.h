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
 // RIP.h
 //

#pragma once

#include "Type.h"
#include "IDP.h"

namespace xns::rip {

void initialize();

class Type : public UINT16 {
    static inline const char* group = "xns::rip::Type";
    Type(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Type() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    
    static UINT16 REQUEST;   // 1
    static UINT16 RESPONSE;  // 2
};
class Delay : public UINT16 {
    static inline const char* group = "xns::rip::Delay";
    Delay(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Delay() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    
    static UINT16 INFINITY;   // 16
};

struct NetDelay {
    Net   net;
    Delay delay;

    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb) {
        net.fromByteBuffer(bb);
        delay.fromByteBuffer(bb);
    }
    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const {
        net.toByteBuffer(bb);
        delay.toByteBuffer(bb);
    }
};

struct RIP {
    Type                  type;
    std::vector<NetDelay> table;

    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb);
    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const;
};

}
