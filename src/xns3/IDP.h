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

class Checksum {
    Checksum() = delete;
    inline static const char* FORMAT = "%04X";
public:
    using T = uint16_t;

    DECL_CLASS_CONSTANT(Checksum, NOCHECK,  0xFFFF)

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

class Type {
    Type() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint8_t;

    DECL_CLASS_CONSTANT(Type, RIP,    1)
    DECL_CLASS_CONSTANT(Type, ECHO,   2)
    DECL_CLASS_CONSTANT(Type, ERROR_, 3)
    DECL_CLASS_CONSTANT(Type, PEX,    4)
    DECL_CLASS_CONSTANT(Type, SPP,    5)
    DECL_CLASS_CONSTANT(Type, BOOT,   6)

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

class Socket {
    Socket() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint16_t;

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

uint16_t computeChecksum(const uint8_t* data, int start, int endPlusOne);
inline uint16_t computeChecksum(const ByteBuffer& bb, int position) { // position point to checksum field of IDP packet
    return computeChecksum(bb.data(), position + 2, bb.limit());
}

struct IDP : Base {
    static constexpr int HEADER_LENGTH = 30;

    uint16_t checksum;  // Checksum
    uint16_t length;
    uint8_t  control;
    uint8_t  type;      // Type

    uint32_t dstNet;    // Net
    uint64_t dstHost;   // Host
    uint16_t dstSocket; // Soeckt

    uint32_t srcNet;    // Net
    uint64_t srcHost;   // Host
    uint16_t srcSocket; // Socket

    IDP(ByteBuffer& bb) {
        fromByteBuffer(bb);
    }
    IDP() {}

    std::string toString() const {
        auto dst = std_sprintf("%s-%s-%s", Net::toString(dstNet), Host::toString(dstHost), Socket::toString(dstSocket));
        auto src = std_sprintf("%s-%s-%s", Net::toString(srcNet), Host::toString(srcHost), Socket::toString(srcSocket));
       return std_sprintf("{%s  %d  %d  %s  %s  %s}",
            Checksum::toString(checksum), length, control, Type::toString(type), dst, src);
    }
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(checksum);
        bb.read16(length);
        bb.read8(control);
        bb.read8(type);
        bb.read32(dstNet);
        bb.read48(dstHost);
        bb.read16(dstSocket);
        bb.read32(srcNet);
        bb.read48(srcHost);
        bb.read16(srcSocket);
    }
    void toByteBuffer(ByteBuffer& bb) const {
        bb.write16(checksum);
        bb.write16(length);
        bb.write8(control);
        bb.write8(type);
        bb.write32(dstNet);
        bb.write48(dstHost);
        bb.write16(dstSocket);
        bb.write32(srcNet);
        bb.write48(srcHost);
        bb.write16(srcSocket);
    }
};

void process(IDP& receive, IDP& transmit);

}