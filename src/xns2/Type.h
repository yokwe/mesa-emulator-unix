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

#include "../util/ByteBuffer.h"
#include "../util/Util.h"

namespace xns {

void initialize();

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
    struct Key {
        const std::string group;
        const T           value;
        Key(const std::string& group_, T value_) : group(group_), value(value_) {}
        std::strong_ordering operator<=>(const Key& that) const {
            return (this->value == that.value) ? (this->group <=> that.group) : (this->value <=> that.value);
        }
    };
    static inline std::map<Key, std::string> formatMap;
    void applyGroup(const char* group) {
        Key key{group, value};
        if (formatMap.contains(key)) {
            this->format   = formatMap[key];
            this->readOnly = true;
        }
    }

    T           value;
    std::string format;
    bool        readOnly;

    BaseNumber(const char* group, T value_, const char* format_) : value(value_), format(format_), readOnly(true) {
        Key key{group, value};
        formatMap[key] = format;
    }

    BaseNumber(const char* format_) : value(0), format(format_), readOnly(false) {}
    virtual ~BaseNumber() {}
public:
    struct FormatEntry {
        std::string group;
        T           value;
        std::string format;
        FormatEntry(std::string group_, T value_, std::string format_) : group(group_), value(value_), format(format_) {}
    };
    static std::vector<FormatEntry> getFormatList() {
        std::vector<FormatEntry> ret;
        for(auto& i: formatMap) {
            FormatEntry entry{i.first.group, i.first.value, i.second};
            ret.push_back(entry);
        }
        return ret;
    }

    // copy constructor
    BaseNumber(const BaseNumber& that) : value(that.value), format(that.format), readOnly(that.readOnly) {}
    // copy assignment operator
    BaseNumber<T>& operator = (const BaseNumber<T>& that) {
        this->value    = that.value;
        this->format   = that.format;
        this->readOnly = that.readOnly;
        return *this;
    }

    std::strong_ordering operator<=>(const BaseNumber<T>& that) const {
        return this->value <=> that.value;
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

    T toValue() const {
        return value;
    }
    const char* toFormat() const {
        return format.c_str();
    }
    // Base
    std::string toString() const {
        return std_sprintf(toFormat(), value);
    }
};
void dumpFormatList();

//
// UINTx
//
class UINT8 : public BaseNumber<uint8_t> {
    using T = uint8_t;
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT8(const char* format_) : BaseNumber<T>(format_) {}
    UINT8(const char* group, T value_, const char* format_) : BaseNumber<T>(group, value_, format_) {} // for enum
public:
    UINT8() : BaseNumber<T>(DEFAULT_FORMAT) {}

    // need to define operator = here
    T operator = (T newValue) {
        return BaseNumber<T>::operator=(newValue);
    }
    // Base
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read8(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write8(value);
    }
    // BaseNumber
    void fromByteBufferGroup(const char* group, ByteBuffer& bb) {
        UINT8::fromByteBuffer(bb);
        //
        applyGroup(group);
    }
};
class UINT16 : public BaseNumber<uint16_t> {
    using T = uint16_t;
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT16(const char* format_) : BaseNumber<T>(format_) {}
    UINT16(const char* group, T value_, const char* format_) : BaseNumber<T>(group, value_, format_) {} // for enum
public:
    UINT16() : BaseNumber<T>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (T newValue) {
        return BaseNumber<T>::operator=(newValue);
    }
    // Base
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read16(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write16(value);
    }
    // BaseNumber
    void fromByteBufferGroup(const char* group, ByteBuffer& bb) {
        UINT16::fromByteBuffer(bb);
        //
        applyGroup(group);
    }
};
class UINT32 : public BaseNumber<uint32_t> {
    using T = uint32_t;
    static inline const char* DEFAULT_FORMAT = "%u";
protected:
    UINT32(const char* format_) : BaseNumber<T>(format_) {}
    UINT32(const char* group, T value_, const char* format_) : BaseNumber<T>(group, value_, format_) {} // for enum
public:
    UINT32() : BaseNumber<T>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (T newValue) {
        return BaseNumber<T>::operator=(newValue);
    }
    // Base
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read32(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write32(value);
    }
    // BaseNumber
    void fromByteBufferGroup(const char* group, ByteBuffer& bb) {
        UINT32::fromByteBuffer(bb);
        //
        applyGroup(group);
    }
};
class UINT48 : public BaseNumber<uint64_t> {
    using T = uint64_t;
    static inline const char* DEFAULT_FORMAT = "%012lX";
protected:
protected:
    UINT48(const char* format_) : BaseNumber<T>(format_) {}
    UINT48(const char* group, T value_, const char* format_) : BaseNumber<T>(group, value_, format_) {} // for enum
public:
    UINT48() : BaseNumber<T>(DEFAULT_FORMAT) {}
    // need to define operator = here
    long operator = (T newValue) {
        return BaseNumber<T>::operator=(newValue);
    }
    // Base
    void fromByteBuffer(ByteBuffer& bb) {
        bb.read48(value);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write48(value);
    }
    // BaseNumber
    void fromByteBufferGroup(const char* group, ByteBuffer& bb) {
        UINT48::fromByteBuffer(bb);
        //
        applyGroup(group);
    }
};
//
// INTx
//
class INT16 : public BaseNumber<int16_t> {
    using T = int16_t;
    static inline const char* DEFAULT_FORMAT = "%d";
protected:
    INT16(const char* format_) : BaseNumber<T>(format_) {}
public:
    INT16() : BaseNumber<T>(DEFAULT_FORMAT) {}
    // need to define operator = here
    int operator = (T newValue) {
        return BaseNumber<T>::operator=(newValue);
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
    using T = uint16_t;
public:
    BOOLEAN() : BaseNumber<T>("") {}

    // define operator =
    bool operator = (bool newValue) {
        return BaseNumber<T>::operator=(newValue ? 1 : 0);
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
        return buffer.toString(buffer.base(), buffer.limit());
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

//
// Host
//
class Host : public UINT48 {
    using T = uint64_t;
    static inline const char* group = "xns::ethernet::Host";
public:
    Host(T value_, const char* name_) : UINT48(group, value_, name_) {}
    Host() : UINT48("%12lX") {}

    long operator = (T newValue) {
        return UINT48::operator=(newValue);
    }

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT48 BROADCAST;
    static UINT48 UNKNOWN;
    static UINT48 BFN_GVWIN;  // Boot File Number of GVWin
    static UINT48 SELF;
};
//
// Net
//
class Net : public UINT32 {
    static inline const char* group = "xns::idp::Net";
public:
    Net(uint32_t value_, const char* name_) : UINT32(group, value_, name_) {}
    Net() : UINT32("%8X") {}

    uint32_t operator = (uint32_t that) {
        UINT32::operator =(that);
        return that;
    }

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    static UINT32 ALL;
    static UINT32 UNKNOWN;
};

namespace host {
    std::string toOctalString(uint64_t value);
    std::string toDecimalString(uint64_t value);
    std::string toHexaDecimalString(uint64_t value, const std::string& sep);

    uint64_t fromString(const std::string& string);
}

}