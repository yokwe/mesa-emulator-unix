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

#include "../util/ByteBuffer.h"
#include "../util/Util.h"

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

template <typename T>
class BaseNumber : public Base {
    static_assert(std::is_integral<T>::value, "T is not number type");
protected:
    T           value;
    const char* format;
    bool        readOnly;
    BaseNumber(T value_, const char* format_) : value(value_), format(format_), readOnly(true) {}
    BaseNumber(const char* format_) : value(0), format(format_), readOnly(false) {}
public:
    // copy constructor
    BaseNumber(const BaseNumber& that) : value(that.value), format(that.format), readOnly(that.readOnly) {}
    // copy assignment operator
    BaseNumber<T>& operator = (const BaseNumber<T>& that) {
        this->value    = that.value;
        this->format   = that.format;
        this->readOnly = that.readOnly;
        return *this;
    }

    T operator = (T newValue) {
        if (readOnly) ERROR();
        return value = newValue;
    }

    // cast operator to number type
    operator T() const noexcept {
        return value;
    }
    // convenience for numeric expression
    T operator +() const noexcept {
        return value;
    }
    // conveniecne for string expression
    std::string operator-() const noexcept {
        return toString();
    }

    // 
    // Base
    //
    std::string toString() const {
        return std_sprintf(format, value);
    }
};
//
// UINTx
//
class UINT8 : public BaseNumber<uint8_t> {
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT8(const char* format_) : BaseNumber<uint8_t>(format_) {}
public:
    UINT8(uint8_t value_, const char* format_) : BaseNumber<uint8_t>(value_, format_)  {} // for enum
    UINT8() : BaseNumber<uint8_t>(DEFAULT_FORMAT)  {}

    // need to define operator = here
    uint8_t operator = (uint8_t newValue) {
        return BaseNumber<uint8_t>::operator=(newValue);
    }
    //
    // Base
    //
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read8(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write8(value);
    }
};
class UINT16 : public BaseNumber<uint16_t> {
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT16(const char* format_) : BaseNumber<uint16_t>(format_) {}
    UINT16(uint16_t value_, const char* format_) : BaseNumber<uint16_t>(value_, format_) {} // for enum
public:
    UINT16() : BaseNumber<uint16_t>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (uint16_t newValue) {
        return BaseNumber<uint16_t>::operator=(newValue);
    }
    //
    // Base
    //
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write16(value);
    }
};
class UINT32 : public BaseNumber<uint32_t> {
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT32(const char* format_) : BaseNumber<uint32_t>(format_) {}
    UINT32(uint32_t value_, const char* format_) : BaseNumber<uint32_t>(value_, format_) {} // for enum
public:
    UINT32() : BaseNumber<uint32_t>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (uint32_t newValue) {
        return BaseNumber<uint32_t>::operator=(newValue);
    }
    //
    // Base
    //
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read32(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write32(value);
    }
};
class UINT48 : public BaseNumber<uint64_t> {
    static inline const char* DEFAULT_FORMAT = "%08lX";
protected:
    UINT48(const char* format_) : BaseNumber<uint64_t>(format_) {}
    UINT48(uint64_t value_, const char* format_) : BaseNumber<uint64_t>(value_, format_)  {} // for enum
public:
    UINT48() : BaseNumber<uint64_t>(DEFAULT_FORMAT) {}
    // need to define operator = here
    long operator = (uint64_t newValue) {
        return BaseNumber<uint64_t>::operator=(newValue);
    }

    //
    // Base
    //
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read48(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write48(value);
    }
};
//
// INTx
//
class INT16 : public BaseNumber<int16_t> {
    static inline const char* DEFAULT_FORMAT = "%d";
protected:
    INT16(const char* format_) : BaseNumber<int16_t>(format_) {}
public:
    INT16() : BaseNumber<int16_t>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (int16_t newValue) {
        return BaseNumber<int16_t>::operator=(newValue);
    }
    //
    // Base
    //
    void fromByteBuffer(ByteBuffer& bb) {
        uint16_t temp;
        bb.read16(temp);
        value = temp;
    }
    void toByteBuffer (ByteBuffer& bb) const {
        uint16_t temp = value;
        bb.write16(temp);
    }
};
//
// BOOLEAN
//
class BOOLEAN : public BaseNumber<uint16_t> {
public:
    BOOLEAN() : BaseNumber<uint16_t>("") {}

    // define operator =
    bool operator = (bool newValue) {
        return BaseNumber<uint16_t>::operator=(newValue ? 1 : 0);
    }

    //
    // Base
    //
    std::string toString() const {
        return value ? "true" : "false";
    }
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write16(value);
    }
};
//
// BLOCK
//
class BLOCK : public Base {
protected:
    ByteBuffer buffer;
public:
    BLOCK() : buffer() {}
    BLOCK(const BLOCK& that) : buffer(that.buffer) {}
    BLOCK operator =(const BLOCK& that) {
        buffer = that.buffer;
        return *this;
    }

    // with ByteBuffer
    explicit BLOCK(const ByteBuffer& newValue) : buffer(newValue) {}

    ByteBuffer toBuffer() const {
        return buffer;
    }

    bool isNull() {
        return buffer.isNull();
    }

    void updateBufferData(const BLOCK& that) {
        buffer.data(that.buffer.data());
    }
    //
    // Base
    //
    std::string toString() const {
        return buffer.toString();
    }
    void fromByteBuffer(ByteBuffer& bb) {
        buffer = bb.newBase();
    }
    void toByteBuffer  (ByteBuffer& bb) const {
        int size = buffer.limit() - buffer.base();
        uint8_t* data = buffer.data() + buffer.base();
        bb.write(size, data);
    }
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


}