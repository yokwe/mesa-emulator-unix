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
// EthernetPacket.h
//

#pragma once

#include "ByteBuffer.h"

class EthernetPacket : public ByteBuffer {
public:
    static constexpr int SIZE = 6 + 6 + 2 + 1500; // 1514

    EthernetPacket() : ByteBuffer(sizeof(packetData), packetData) {
        memset(packetData, 0, sizeof(packetData));
    }
    ~EthernetPacket() {}

    // Packet
    EthernetPacket(const EthernetPacket& that) {
        copyFrom(that);
    }
    EthernetPacket& operator =(const EthernetPacket& that) {
        copyFrom(that);
        return *this;
    }

    // ByteBuffer
    EthernetPacket(const ByteBuffer& that) : ByteBuffer() {
        copyFrom(that);
    }
    EthernetPacket& operator =(const ByteBuffer& that) {
        copyFrom(that);
        return *this;
    }
private:
    uint8_t packetData[SIZE];
    
    void copyFrom(const ByteBuffer& that) {
        // copy values from that
        myBase     = that.base();
        myPosition = that.position();
        myLimit    = that.limit();
        // use packetData for myData
        myCapacity = sizeof(packetData);
        myData     = packetData;
        // reset myMarkPos
        myMarkPos  = INVALID_POS;
        // copy data from that to myPacketData
        memset(packetData, 0, sizeof(packetData));
        memcpy(packetData, that.data(), that.capacity());
    }
};
