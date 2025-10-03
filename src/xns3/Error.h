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
 // Error.h
 //

#pragma once

#include <cstdint>

#include "../util/ByteBuffer.h"

#include "Type.h"

namespace xns::error {

void initialize();

class ErrorNumber {
    ErrorNumber() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint16_t;

    DECL_CLASS_CONSTANT(ErrorNumber, UNSPEC,               0)
    DECL_CLASS_CONSTANT(ErrorNumber, BAD_CHECKSUM,         1)
    DECL_CLASS_CONSTANT(ErrorNumber, NO_SOCKET,            2)
    DECL_CLASS_CONSTANT(ErrorNumber, RESOURCE_LIMIT,       3)

    DECL_CLASS_CONSTANT(ErrorNumber, LISTEN_REJECT,        4)
    DECL_CLASS_CONSTANT(ErrorNumber, INVALID_PACKET_TYPE,  5)
    DECL_CLASS_CONSTANT(ErrorNumber, PROTOCOL_VIOLATION,   6)

    DECL_CLASS_CONSTANT(ErrorNumber, UNSPECIFIED_IN_ROUTE, 01000)
    DECL_CLASS_CONSTANT(ErrorNumber, INCONSISTENT,         01001)
    DECL_CLASS_CONSTANT(ErrorNumber, CANT_GET_THERE,       01002)
    DECL_CLASS_CONSTANT(ErrorNumber, EXCESS_HOPS,          01003)
    DECL_CLASS_CONSTANT(ErrorNumber, TOO_BIG,              01004)

    DECL_CLASS_CONSTANT(ErrorNumber, CONGESTION_WARNING,   01005)
    DECL_CLASS_CONSTANT(ErrorNumber, CONGESTION_DISCARD,   01006)

    static std::string toString(T value) {
        return constantMap.toString(value);
    }
    static void registerName(T value, const std::string& name) {
        constantMap.registerName(value, name);
    }
private:
    struct MyConstantMap: public ConstantMap<T> {
        MyConstantMap() : ConstantMap<T>(FORMAT) {
            initialize();
        }
        void initialize();
    };

    static inline MyConstantMap constantMap;
};


struct Error : Base {
    uint16_t errorNumber;
    uint16_t errorParameter;

    Error() {}

    std::string toString() const {
        return std_sprintf("{%s  %d}", ErrorNumber::toString(errorNumber), errorParameter);
    }

    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(errorNumber);
        bb.read16(errorParameter);
    }
    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const {
        bb.write16(errorNumber);
        bb.write16(errorParameter);
    }

    Error(ByteBuffer& bb) {
        fromByteBuffer(bb);
    }
    Error(uint16_t errorNumber_, uint16_t errorParameter_) : errorNumber(errorNumber_), errorParameter(errorParameter_) {}
};

}
