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
 // Ethernet.h
 //

#pragma once

#include "Type.h"

namespace xns::ethernet {

void initialize();

class Host : public UINT48 {
    static inline const char* group = "xns::ethernet::Host";
    Host(uint64_t value_, const char* name_) : UINT48(group, value_, name_) {}
public:
    Host() : UINT48("%12lX") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write48(value);
    }

    static UINT48 BROADCAST;
    static UINT48 UNKNOWN;
    static UINT48 BFN_GVWIN;  // Boot File Number of GVWin
    static UINT48 SELF;
};

class Type : public UINT16 {
    static inline const char* group = "xns::ethernet::Type";
    Type(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Type() : UINT16("%04X") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write48(value);
    }

    static UINT16 XNS;
    static UINT16 IP;
};

struct Frame {
    Host  dest;
    Host  source;
    Type  type;
    BLOCK block;

    void fromByteBuffer(ByteBuffer& bb) {
        dest.fromByteBuffer(bb);
        source.fromByteBuffer(bb);
        type.fromByteBuffer(bb);
        block.fromByteBuffer(bb);
    }
};

}
