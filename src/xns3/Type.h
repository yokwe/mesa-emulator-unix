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
 // Type.h
 //

#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include "../util/Util.h"

#include "../util/net.h"
#include "../util/ByteBuffer.h"

namespace xns {

// helper macro to invoke fromByteBuufer / toByteBuffer
#define FROM_BYTE_BUFFER(bb, name) name.fromByteBuffer(bb)
#define TO_BYTE_BUFFER(bb, name) name.toByteBuffer(bb)

class Base {
public:
    virtual ~Base() {}

    // std::string <= this
    virtual std::string toString() const = 0;
    // this <= ByteBuffer
    virtual void fromByteBuffer(ByteBuffer& bb) = 0;
    // ByteBuffer <= this
    virtual void toByteBuffer(ByteBuffer& bb) const = 0;
};


//
// STRING
//
class STRING : public Base {
    static constexpr int MAX_LENGTH = 65535;
    mutable std::string string;
public:
    // define operator =
    std::string operator =(const std::string& newValue) const {
        return string = newValue;
    }

    // cast to const char*
    operator const char* () {
        return string.c_str();
    }
    //
    // Base
    //
    std::string toString() const {
        return string;
    }
    void fromByteBuffer(ByteBuffer& bb);
    void toByteBuffer  (ByteBuffer& bb) const;
};
//
// ARRAY
//
template <typename T, uint16_t N>
class ARRAY : public Base {
    static_assert(std::is_base_of<Base, T>::value, "T is not derived from Base");
    int length = N;
    mutable std::vector<T> list;

public:
    ARRAY() {}

    ARRAY(const std::vector<T>& that) {
        if (length != that.length()) {
            logger.error("Unexpected");
            logger.error("  length %d", length);
            logger.error("  that   %d", that.length());
            ERROR();
        }
        list = that;
    };
    ARRAY& operator = (const std::vector<T>& that) const {
        if (length != that.length()) {
            logger.error("Unexpected");
            logger.error("  length %d", length);
            logger.error("  that   %d", that.length());
            ERROR();
        }
        list = that;
        return *this;
    }

    int size() const {
        return list.size();
    }
    void clear() {
        list.clear();
    }
    void append(T& newValue) {
        list.push_back(newValue);
    }
    void append(std::vector<T> newValue) {
        for(auto e: newValue) {
            list.append(e);
        }
    }
    //
    // Base
    //
    std::string toString() const {
        if (list.isEmpty()) {
            return "{}";
        } else {
            std::string string;
            for(auto e: list) {
                string += std_sprintf(", {%s}", e.toString());
            }
            return std_sprintf("(%d)-(%s))", list.size(), string.substr(2));
        }
    }
    void fromByteBuffer(ByteBuffer& bb) {
        list.clear();
        for(int i = 0; i < length; i++) {
            T newValue;
            newValue.fromByteBuffer(bb);
            list.push_back(newValue);
        }
    }
    void toByteBuffer  (ByteBuffer& bb) const {
        for(auto e: list) {
            e.toByteBuffer(bb);
        }
    }
};
//
// SEQUENCE
//
template <typename T, uint16_t N = 65535>
class SEQUENCE : public Base {
    static_assert(std::is_base_of<Base, T>::value, "T is not derived from Base");
    uint16_t  maxLength = N;
    mutable std::vector<T> list;

public:
    SEQUENCE() {}

    SEQUENCE(const std::vector<T>& that) {
        if (maxLength < that.length()) {
            logger.error("Unexpected");
            logger.error("  maxLength %d", maxLength);
            logger.error("  that      %d", that.length());
            ERROR();
        }
        list = that;
    };
    SEQUENCE& operator = (const std::vector<T>& that) const {
        if (maxLength < that.length()) {
            logger.error("Unexpected");
            logger.error("  maxLength %d", maxLength);
            logger.error("  that      %d", that.length());
            ERROR();
        }
        list = that;
        return *this;
    }

    int size() const {
        return list.size();
    }
    void clear() {
        list.clear();
    }
    void append(T& newValue) {
        list.append(newValue);
    }
    void append(std::vector<T> newValue) {
        for(auto e: newValue) {
            list.push_back(e);
        }
    }
    //
    // Base
    //
    std::string toString() const {
        std::string string;
        for(auto e: list) {
            string += std_sprintf(", {%s}", e.toString());
        }
        return std_sprintf("(%d)-(%s))", list.size(), string.substr(2));
    }
    void fromByteBuffer(ByteBuffer& bb) {
        uint16_t length;
        bb.read16(length);
        if (maxLength < length) {
            logger.error("Unexpected");
            logger.error("  maxLength = %u", maxLength);
            logger.error("  length    = %u", length);
            ERROR();
        }
        list.clear();
        for(uint16_t i = 0; i < length; i++) {
            T newValue;
            newValue.fromByteBuffer(bb);
            list.push_back(newValue);
        }
    }
    void toByteBuffer  (ByteBuffer& bb) const {
        bb.write16((uint16_t)list.length());
        for(auto e: list) {
            e.toByteBuffer(bb);
        }
    }
};



//
// utiilty class
//
template<typename T>
class ConstantMap {
protected:
    const char* defaultFormat;
    std::map<T, std::string> map;

public:
    ConstantMap(const char* defaultFormat_) : defaultFormat(defaultFormat_) {}
    virtual ~ConstantMap() {}

    virtual void initialize() = 0;

    bool contains(T value) {
        return map.contains(value);
    }
    std::string toString(T value) {
        return map.contains(value) ? map[value] : std_sprintf(defaultFormat, value);
    }
    void registerName(T value, const std::string& name) {
        map[value] = name;
    }
};

#define DECL_CLASS_CONSTANT(type, name, value) static inline constexpr T name = value;

//
// Host
//
class Host {
    Host() = delete;
    inline static const char* FORMAT = "%012lX";
public:
    using T = uint64_t;

    DECL_CLASS_CONSTANT(Host, BROADCAST, 0xFFFF'FFFF'FFFF)
    DECL_CLASS_CONSTANT(Host, UNKNOWN,   0x0000'0000'0000)
    DECL_CLASS_CONSTANT(Host, BFN_GVWIN, 0x0000'aa00'0e60)

    static std::string toString(T value) {
        return constantMap.contains(value) ? constantMap.toString(value) : net::toHexaDecimalString(value);
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

//
// Net
//
class Net {
    Net() = delete;
    inline static const char* FORMAT = "%d";
public:
    using T = uint32_t;

    DECL_CLASS_CONSTANT(Net, ALL,     0xFFFF'FFFF)
    DECL_CLASS_CONSTANT(Net, UNKNOWN, 0x0000'0000)

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

}