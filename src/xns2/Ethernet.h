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

class Type : public UINT16 {
    static inline const char* group = "xns::ethernet::Type";
    Type(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Type() : UINT16("%04X") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT16 XNS;
    static UINT16 IP;
};

struct Frame {
    static constexpr int HEADER_LENGTH  = 14;
    static constexpr int MINIMU_LENGTH  = 64;
    static constexpr int MAXIMUM_LENGTH = 6 + 6 + 2 + 1500; // 1514

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
    void toByteBuffer(ByteBuffer& bb) {
        int position = bb.position();
        int limit    = bb.limit();
        int length   = limit - position;

        dest.toByteBuffer(bb);
        source.toByteBuffer(bb);
        type.toByteBuffer(bb);
        block.toByteBuffer(bb);

        // add padding if necessary
        if (length < MINIMU_LENGTH) {
            int padding = MINIMU_LENGTH - length;
            for(int i = 0; i < padding; i++) bb.write8(0);
        }
    }
};


void processRequest(const Frame& request, ByteBuffer& response);

}
