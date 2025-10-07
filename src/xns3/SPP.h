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
 // SPP.h
 //

#pragma once

#include "Type.h"

namespace xns::spp {

struct SPP : public Base {
    uint8_t  control; // Control Bit
    uint8_t  sst;     // Sub System Type
    uint16_t idSrc;   // connection id of source
    uint16_t idDst;   // connection id of destination
    uint16_t seq;     // sequence
    uint16_t ack;     // acknowledgment
    uint16_t alloc;   // allocation

    std::string toString() const {
        return std_sprintf("{%02X  %d  %04X  %04X  %5d  %5d  %5d}", control, sst, idSrc, idDst, seq, ack, alloc);
    }
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read8(control);
        bb.read8(sst);
        bb.read16(idSrc);
        bb.read16(idDst);
        bb.read16(seq);
        bb.read16(ack);
        bb.read16(alloc);
    }
    void toByteBuffer  (ByteBuffer& bb) const {
        bb.write8(control);
        bb.write8(sst);
        bb.write16(idSrc);
        bb.write16(idDst);
        bb.write16(seq);
        bb.write16(ack);
        bb.write16(alloc);
    }
};

}
