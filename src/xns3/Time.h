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

class Version {
    Version() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint16_t;

    DECL_CLASS_CONSTANT(Type, CURRENT,  2)

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
    using T = uint16_t;

    DECL_CLASS_CONSTANT(Type, REQUEST,  1)
    DECL_CLASS_CONSTANT(Type, RESPONSE, 2)

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

class Direction {
    Direction() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint16_t;

    DECL_CLASS_CONSTANT(Direction, WEST, 0)
    DECL_CLASS_CONSTANT(Direction, EAST, 1)

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

class Tolerance {
    Tolerance() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint16_t;

    DECL_CLASS_CONSTANT(Tolerance, UNKNOWN, 0)
    DECL_CLASS_CONSTANT(Tolerance, KNOWN,   1)

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


struct Request : public Base {
    uint16_t   version; // Version
    uint16_t   type;    // Type

    Request(ByteBuffer& bb) {
        fromByteBuffer(bb);
    }

    std::string toString() const {
        return std_sprintf("{%s  %s}", Version::toString(version), Type::toString(type));
    }

    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(version);
        bb.read16(type);
    }
    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const {
        bb.write16(version);
        bb.write16(type);
    }
};

std::string toStringLocalTime(uint32_t time);

struct Response : public Base {
    uint16_t version;          // Version
    uint16_t type;             // Type
    uint32_t time;             // current time between 12:00:00, 1 Jan. 1968 and 6:28:23, 6 Feb. 2104 inclusive
    uint16_t offsetDirection;  // Direction
    uint16_t offsetHours;
    uint16_t offsetMinutes;
    uint16_t dstStart;         // 0 for no DST
    uint16_t dstEnd;           // 0 for no DST
    uint16_t tolerance;        // 0 for UNKNOWN  1 for KNOWN
    uint32_t toleranceValue;   // supposed time error in unit of millisecond

    std::string toString() const {
        uint32_t unixTime = Util::toUnixTime(time);
        std::string timeString = toStringLocalTime(unixTime);
        return std_sprintf("{%s  %s  %s  %s  %dh%dm  %d-%d  %s-%d}",
            Version::toString(version), Type::toString(type), timeString,
            Direction::toString(offsetDirection), offsetHours, offsetMinutes, dstStart, dstEnd,
            Tolerance::toString(tolerance), toleranceValue);
    }
    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(version);
        bb.read16(type);
        bb.read32(time);
        bb.read16(offsetDirection);
        bb.read16(offsetHours);
        bb.read16(offsetMinutes);
        bb.read16(dstStart);
        bb.read16(dstEnd);
        bb.read16(tolerance);
        bb.read32(toleranceValue);
    }
    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const {
        bb.write16(version);
        bb.write16(type);
        bb.write32(time);
        bb.write16(offsetDirection);
        bb.write16(offsetHours);
        bb.write16(offsetMinutes);
        bb.write16(dstStart);
        bb.write16(dstEnd);
        bb.write16(tolerance);
        bb.write32(toleranceValue);
    }
};

}