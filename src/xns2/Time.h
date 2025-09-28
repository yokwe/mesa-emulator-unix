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
 // Time.h
 //

#pragma once

#include "Type.h"

namespace xns::time {

void initialize();

class Version : public UINT16 {
    static inline const char* group = "xns::time::Version";
    Version(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Version() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT16 CURRENT;
};

class Type : public UINT16 {
    static inline const char* group = "xns::time::Type";
    Type(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Type() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    
    static UINT16 REQUEST;
    static UINT16 RESPONSE;
};

class Direction : public UINT16 {
    static inline const char* group = "xns::time::Direction";
    Direction(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Direction() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }

    static UINT16 WEST;
    static UINT16 EAST;
};

class Tolerance : public UINT16 {
    static inline const char* group = "xns::time::Tolerance";
    Tolerance(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    Tolerance() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }

    static UINT16 UNKNOWN; // 0
    static UINT16 KNOWN;  // 1
};


struct Request {
    Version   version;
    Type      type;

    void fromByteBuffer(ByteBuffer& bb) {
        version.fromByteBuffer(bb);
        type.fromByteBuffer(bb);
    }
};

struct Response {
    Version   version;
    Type      type;
    UINT32    time;             // current time between 12:00:00, 1 Jan. 1968 and 6:28:23, 6 Feb. 2104 inclusive
    Direction offsetDirection;  // east or west of prime meridian
    UINT16    offsetHours;
    UINT16    offsetMinutes;
    UINT16    dstStart;         // 0 for no DST
    UINT16    dstEnd;           // 0 for no DST
    Tolerance tolerance;    // 0 for 
    UINT32    toleranceValue;   // supposed time error in unit of tolerance

    void toByteBuffer(ByteBuffer& bb) {
        version.toByteBuffer(bb);
        type.toByteBuffer(bb);
        time.toByteBuffer(bb);
        offsetDirection.toByteBuffer(bb);
        offsetHours.toByteBuffer(bb);
        offsetMinutes.toByteBuffer(bb);
        dstStart.toByteBuffer(bb);
        dstEnd.toByteBuffer(bb);
        tolerance.toByteBuffer(bb);
        toleranceValue.toByteBuffer(bb);
    }
};

}