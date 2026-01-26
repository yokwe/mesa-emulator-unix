/*******************************************************************************
 * Copyright (c) 2026, Yasuhiro Hasegawa
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
// ByteBuffer.h
//

#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>
#include <span>

#include "Util.h"


class ByteBuffer {
    static const int bytesPerWord = 2;

    static uint32_t byteValueToWordValue(uint32_t byteValue) {
        return (byteValue + bytesPerWord - 1) / bytesPerWord;
    }
    static uint32_t wordValueToByteValue(uint32_t wordValue) {
        return wordValue * bytesPerWord;
    }

    struct Impl {
        const char* myName;
        uint32_t    mySize;
        uint8_t*    myData;

        Impl(const char* name) : myName(name), mySize(0), myData(0) {}
        Impl(const char* name, uint32_t size) : myName(name), mySize(size), myData(new uint8_t[mySize]) {}

        virtual ~Impl() {
            delete[] myData;
            myName = 0;
            mySize = 0;
            myData = 0;
        }

        // 32
        virtual uint32_t get32(uint8_t* data, uint32_t pos) = 0;
        virtual void     put32(uint8_t* data, uint32_t pos, uint32_t value) = 0;
    };
    
    static inline const uint32_t BAD_MARK = ~0;

    std::shared_ptr<Impl> myImpl;
    uint8_t*  myData;
    uint32_t  myByteCapacity;  // cpacity of myData
    uint32_t  myBytePos;       // current position for read and write
    uint32_t  myByteLimit;     // written size
    uint32_t  myByteMark;

    // valid range of myBytePos   is [0..myByteCapacity)
    // valid range of myByteLimit is [myBytePos..myByteCapacity)
    void checkBeforeRead(uint32_t byteSize);
    void checkBeforeWrite(uint32_t byteSize);


    ByteBuffer(std::shared_ptr<Impl> impl, uint8_t* data, uint32_t byteCapacity) :
        myImpl(impl), myData(data), myByteCapacity(byteCapacity), myBytePos(0), myByteLimit(0), myByteMark(BAD_MARK) {}
    ByteBuffer(std::shared_ptr<Impl> impl, uint8_t* data, uint32_t byteCapacity, uint32_t byteLimit) :
        myImpl(impl), myData(data), myByteCapacity(byteCapacity), myBytePos(0), myByteLimit(byteLimit), myByteMark(BAD_MARK) {}

    // 8
    uint8_t get8(uint32_t bytePos) {
        return myData[bytePos];
    }
    void put8(uint32_t bytePos, uint8_t value) const {
        myData[bytePos] = value;
    }
    // 16
    uint16_t get16(uint32_t bytePos) {
        return (myData[bytePos + 0] << 8) | (myData[bytePos + 1] << 0);
    }
    void put16(uint32_t bytePos, uint16_t value) const {
        myData[bytePos + 0] = value >> 8;
        myData[bytePos + 1] = value >> 0;
    }
    // 32
    uint32_t get32(uint32_t bytePos) {
        return myImpl->get32(myData, bytePos);
    }
    void put32(uint32_t bytePos, uint32_t value) {
        myImpl->put32(myData, bytePos, value);
    }

public:
    class Mesa {
        static inline const char* NAME = "MESA";
        struct MyImpl : public Impl {
            MyImpl() : Impl(NAME) {}
            MyImpl(uint32_t size) : Impl(NAME, size) {}

            // 32
            // mesa order for 32bit value  low half and high half
            uint32_t get32(uint8_t* data, uint32_t pos) override {
                return (data[pos + 0] << 8) | (data[pos + 1] << 0) | (data[pos + 2] << 24) | (data[pos + 3] << 16);
            }
            void     put32(uint8_t* data, uint32_t pos, uint32_t value) override {
                data[pos + 0] = (uint8_t)(value >>  8);
                data[pos + 1] = (uint8_t)(value >>  0);
                data[pos + 2] = (uint8_t)(value >> 24);
                data[pos + 3] = (uint8_t)(value >> 16);
            }
        };
    public:
        static ByteBuffer getInstance(uint8_t* data, uint32_t size) {
            static std::shared_ptr<MyImpl> myImpl(new MyImpl());
            return ByteBuffer(myImpl, data, size, size);
        }
        static ByteBuffer getInstance(uint32_t size) {
            std::shared_ptr<MyImpl> myImpl(new MyImpl(size));
            return ByteBuffer(myImpl, myImpl->myData, size);
        }
    };
    class Net {
        static inline const char* NAME = "NET";
        struct MyImpl : public Impl {
            MyImpl() : Impl(NAME) {}
            MyImpl(uint32_t size) : Impl(NAME, size) {}

            // 32
            // netowrk order for 32bit value  high half and low half
            uint32_t get32(uint8_t* data, uint32_t pos) override {
                return (data[pos + 0] << 24) | (data[pos + 1] << 16) | (data[pos + 2] << 8) | (data[pos + 3] << 0);
            }
            void     put32(uint8_t* data, uint32_t pos, uint32_t value) override {
                data[pos + 0] = (uint8_t)(value >> 24);
                data[pos + 1] = (uint8_t)(value >> 16);
                data[pos + 2] = (uint8_t)(value >>  8);
                data[pos + 3] = (uint8_t)(value >>  0);
            }
        };
    public:
        static ByteBuffer getInstance() {
            static std::shared_ptr<MyImpl> myImpl(new MyImpl());
            return ByteBuffer(myImpl, 0, 0);
        }
        static ByteBuffer getInstance(uint8_t* data, uint32_t size) {
            static std::shared_ptr<MyImpl> myImpl(new MyImpl());
            return ByteBuffer(myImpl, data, size, size);
        }
        static ByteBuffer getInstance(uint32_t size) {
            std::shared_ptr<MyImpl> myImpl(new MyImpl(size));
            return ByteBuffer(myImpl, myImpl->myData, size);
        }
    };

    static uint8_t highByte(uint16_t value) {
        return (uint8_t)(value >> 8);
    }
    static uint8_t lowByte(uint16_t value) {
        return (uint8_t)(value >> 0);
    }


    ~ByteBuffer() {
        myData          = 0;
        myByteCapacity  = 0;
        myBytePos       = 0;
        myByteLimit     = 0;
        myByteMark      = BAD_MARK;
    }

    ByteBuffer byteRange(uint32_t byteOffset, uint32_t byteSize) const;
    ByteBuffer range(uint32_t wordOffset, uint32_t wordSize) const {
        auto byteOffset  = wordValueToByteValue(wordOffset);
        auto byteSize = wordValueToByteValue(wordSize);
        return byteRange(byteOffset, byteSize);
    }
    ByteBuffer rangeRemains() const {
        return byteRange(myBytePos, byteRemains());
    }

    std::span<uint8_t> toSpan() const {
        return std::span<uint8_t>{myData, myByteLimit};
    }

    std::string toString() {
        return toHexString(myByteLimit, myData);
    }

    const char* name() const {
        return myImpl->myName;
    }

    const uint8_t* data() const {
        return myData;
    }

    //
    // clear -- make ByteBufer ready for write
    //
    void clear() {
        myByteLimit = 0;
        myBytePos   = 0;
    }
    //
    // flip -- make ByteBuffer ready for read from start after write
    //
    void flip() {
        myByteLimit = myBytePos;
        myBytePos   = 0;
    }
    //
    // rewind -- make ByteBuffer ready for read from start
    //
    void rewind() {
        myBytePos = 0;
    }
    //
    // mark -- mark position for later reset
    //
    void mark();
    //
    // reset -- move to last mark position
    //
    void reset();


    //
    // capacity
    //
    uint32_t byteCapacity() const {
        return myByteCapacity;
    }
    uint32_t capacity() const {
        return byteValueToWordValue(byteCapacity());
    }
    //
    // pos
    //
    uint32_t bytePos() const {
        return myBytePos;
    }
    uint32_t pos() const {
        return byteValueToWordValue(bytePos());
    }
    //
    // limit
    //
    uint32_t byteLimit() const {
        return myByteLimit;
    }
    uint32_t limit() const {
        return byteValueToWordValue(byteLimit());
    }
    bool empty() const {
        return byteLimit() == 0;
    }
    //
    // remains
    //
    uint32_t byteRemains() const {
        return myByteLimit - myBytePos;
    }
    uint32_t remains() const {
        return byteValueToWordValue(byteRemains());
    }

    
    // getX
    uint8_t get8() {
        const uint32_t byteSize = 1;

        checkBeforeRead(byteSize);
        auto ret = get8(myBytePos);
        myBytePos += byteSize;
        return ret;
    }
    uint16_t get16() {
        const uint32_t byteSize = 2;

        checkBeforeRead(byteSize);
        auto ret = get16(myBytePos);
        myBytePos += byteSize;
        return ret;
    }
    uint32_t get32() {
        const uint32_t byteSize = 4;

        checkBeforeRead(byteSize);
        auto ret = get32(myBytePos);
        myBytePos += byteSize;
        return ret;
    }

    // putX
    void put(std::span<uint8_t> span) {
        auto byteSize = span.size();

        checkBeforeWrite(byteSize);
        for(auto e: span) {
            put8(myBytePos++, e);
        }
        if (myByteLimit < myBytePos) myByteLimit = myBytePos;
    }
    void put8(uint8_t value) {
        const uint32_t byteSize = 1;

        checkBeforeWrite(byteSize);
        put8(myBytePos, value);
        myBytePos += byteSize;
        if (myByteLimit < myBytePos) myByteLimit = myBytePos;
    }
    void put16(uint16_t value) {
        const uint32_t byteSize = 2;

        checkBeforeWrite(byteSize);
        put16(myBytePos, value);
        myBytePos += byteSize;
        if (myByteLimit < myBytePos) myByteLimit = myBytePos;
    }
    void put32(uint32_t value) {
        const uint32_t byteSize = 4;

        checkBeforeWrite(byteSize);
        put32(myBytePos, value);
        myBytePos += byteSize;
        if (myByteLimit < myBytePos) myByteLimit = myBytePos;
    }

    //
    // HasRead and read(...)
    //
    struct HasRead {
        virtual ByteBuffer& read(ByteBuffer& bb) = 0;
        virtual ~HasRead() = default;
    };
    ByteBuffer& read() {
        return *this;
    }
    template <class Head, class... Tail>
    ByteBuffer& read(Head&& head, Tail&&... tail) {
        using T = std::remove_cv_t<std::remove_reference_t<Head>>;
        constexpr auto is_uint8_t  = std::is_same_v<T, uint8_t>;
        constexpr auto is_uint16_t = std::is_same_v<T, uint16_t>;
        constexpr auto is_uint32_t = std::is_same_v<T, uint32_t>;
        constexpr auto is_class    = std::is_class_v<T>;
        constexpr auto is_enum     = std::is_scoped_enum_v<T>;

//		logger.info("read head  %2d  |  %d  %d  |  %d  %d  | %s", sizeof(head), is_uint16_t, is_uint32_t, is_class, is_enum, demangle(typeid(head).name()));

        if constexpr (is_uint8_t || is_uint16_t || is_uint32_t) {
            read(head);
        } else if constexpr (is_enum) {
            using UT = std::underlying_type_t<T>;
            constexpr auto ut_uint8_t  = std::is_same_v<UT, uint8_t>;
            constexpr auto ut_uint16_t = std::is_same_v<UT, uint16_t>;
            constexpr auto ut_uint32_t = std::is_same_v<UT, uint32_t>;

//    		logger.info("enum class  %d  %d  %d", ut_uint8_t, ut_uint16_t, ut_uint32_t);
            if constexpr (ut_uint8_t) {
                uint8_t t;
                read(t);
                head = static_cast<std::remove_cv_t<std::remove_reference_t<Head>>>(t);
            } else if constexpr(ut_uint16_t) {
                uint16_t t;
                read(t);
                head = static_cast<std::remove_cv_t<std::remove_reference_t<Head>>>(t);
            } else if constexpr (ut_uint32_t) {
                uint32_t t;
                read(t);
                head = static_cast<std::remove_cv_t<std::remove_reference_t<Head>>>(t);
            } else {
                logger.error("Unexpected type  %s", demangle(typeid(head).name()));
                ERROR()    
            }
        } else if constexpr (is_class) {
            constexpr auto has_read = std::is_base_of_v<HasRead, std::remove_reference_t<Head>>;
            if constexpr (has_read) {
                head.read(*this);
            } else {
                logger.error("Unexpected type  %s", demangle(typeid(head).name()));
                ERROR()
            }
        } else {
            logger.error("Unexpected type  %s", demangle(typeid(head).name()));
            ERROR()
        }
        return read(std::forward<Tail>(tail)...);
    }

    ByteBuffer& read(uint8_t& value) {
        value = get8();
        return *this;
    }
    ByteBuffer& read(uint16_t& value) {
        value = get16();
        return *this;
    }
    ByteBuffer& read(uint32_t& value) {
        value = get32();
        return *this;
    }
    ByteBuffer& read(int      value) = delete;
    ByteBuffer& read(int64_t  value) = delete;
    ByteBuffer& read(uint64_t value) = delete;


    //
    // HasWrite and write(...)
    //
    struct HasWrite {
        virtual ByteBuffer& write(ByteBuffer& bb) const = 0;
        virtual ~HasWrite() = default;
    };
    ByteBuffer& write() {
        return *this;
    }
    template <class Head, class... Tail>
    ByteBuffer& write(Head&& head, Tail&&... tail) {
        using T = std::remove_cv_t<std::remove_reference_t<Head>>;
        constexpr auto is_uint8_t  = std::is_same_v<T, uint8_t>;
        constexpr auto is_uint16_t = std::is_same_v<T, uint16_t>;
        constexpr auto is_uint32_t = std::is_same_v<T, uint32_t>;
        constexpr auto is_class    = std::is_class_v<T>;
        constexpr auto is_enum     = std::is_scoped_enum_v<T>;

//		logger.info("read head  %2d  |  %d  %d  |  %d  %d  | %s", sizeof(head), is_uint16_t, is_uint32_t, is_class, is_enum, demangle(typeid(head).name()));

        if constexpr (is_uint8_t || is_uint16_t || is_uint32_t) {
            write(head);
        } else if constexpr (is_enum) {
            using UT = std::underlying_type_t<T>;
            constexpr auto ut_uint8_t  = std::is_same_v<UT, uint8_t>;
            constexpr auto ut_uint16_t = std::is_same_v<UT, uint16_t>;
            constexpr auto ut_uint32_t = std::is_same_v<UT, uint32_t>;

//    		logger.info("enum class  %d  %d  %d", ut_uint8_t, ut_uint16_t, ut_uint32_t);
            if constexpr (ut_uint8_t) {
                auto t = static_cast<uint8_t>(head);
                write(t);
            } else if constexpr(ut_uint16_t) {
                auto t = static_cast<uint16_t>(head);
                write(t);
            } else if constexpr (ut_uint32_t) {
                auto t = static_cast<uint32_t>(head);
                write(t);
            } else {
                logger.error("Unexpected type  %s", demangle(typeid(head).name()));
                ERROR()    
            }
        } else if constexpr (is_class) {
            constexpr auto has_write = std::is_base_of_v<HasWrite, std::remove_reference_t<Head>>;
            if constexpr (has_write) {
                head.write(*this);
            } else {
                logger.error("Unexpected type  %s", demangle(typeid(head).name()));
                ERROR()
            }
        } else {
            logger.error("Unexpected type  %s", demangle(typeid(head).name()));
            ERROR()
        }
        return write(std::forward<Tail>(tail)...);
    }
    ByteBuffer& write(uint8_t value) {
        put8(value);
        return *this;
    }
    ByteBuffer& write(uint16_t value) {
        put16(value);
        return *this;
    }
    ByteBuffer& write(uint32_t value) {
        put32(value);
        return *this;
    }
    ByteBuffer& write(int      value) = delete;
    ByteBuffer& write(int64_t  value) = delete;
    ByteBuffer& write(uint64_t value) = delete;

};
