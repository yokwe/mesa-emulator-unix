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
#include <type_traits>

#include "../util/Util.h"


class ByteBuffer {
    static const int bytesPerWord = 2;

    static uint32_t byteValueToWordValue(uint32_t byteValue) {
        return (byteValue + bytesPerWord - 1) / bytesPerWord;
    }
    static uint32_t wordValueToByteValue(uint32_t wordValue) {
        return wordValue * bytesPerWord;
    }

    struct Impl {
        virtual const char* name() = 0;

        // 8
        virtual uint8_t  get8(uint8_t* data, uint32_t pos) = 0;
        virtual void     put8(uint8_t* data, uint32_t pos, uint8_t value) = 0;
        // 16
        virtual uint16_t get16(uint8_t* data, uint32_t pos) = 0;
        virtual void     put16(uint8_t* data, uint32_t pos, uint16_t value) = 0;
        // 32
        virtual uint32_t get32(uint8_t* data, uint32_t pos) = 0;
        virtual void     put32(uint8_t* data, uint32_t pos, uint32_t value) = 0;
    };

    Impl&     myImpl;
    uint8_t*  myData;
    uint32_t  myByteSize;
    uint32_t  myBytePos;

    ByteBuffer(Impl& impl, uint8_t* data, uint32_t byteSize) : myImpl(impl), myData(data), myByteSize(byteSize), myBytePos(0) {}

    void checkByteRange(uint32_t bytePos, uint32_t readSize) const {
        checkBytePos(bytePos);
        checkBytePos(bytePos + readSize - 1);
    }
    void checkBytePos(uint32_t pos) const;

    // getX
    uint8_t get8(uint32_t bytePos) {
        return myImpl.get8(myData, bytePos);
    }
    uint16_t get16(uint32_t bytePos) {
        return myImpl.get16(myData, bytePos);
    }
    uint32_t get32(uint32_t bytePos) {
        return myImpl.get32(myData, bytePos);
    }
    // putX
    void put8(uint32_t bytePos, uint8_t value) const {
        myImpl.put8(myData, bytePos, value);
    }
    void put16(uint32_t bytePos, uint16_t value) const {
        myImpl.put16(myData, bytePos, value);
    }
    void put32(uint32_t bytePos, uint32_t value) const {
        myImpl.put32(myData, bytePos, value);
    }
public:
    static uint8_t highByte(uint16_t value) {
        return (uint8_t)(value >> 8);
    }
    static uint8_t lowByte(uint16_t value) {
        return (uint8_t)(value >> 0);
    }

    struct Mesa {
        static inline struct : public Impl {
            const char* name() override {
                return "MESA";
            }
            // 8
            uint8_t  get8(uint8_t* data, uint32_t pos) override {
                return data[pos];
            }
            void     put8(uint8_t* data, uint32_t pos, uint8_t value) override {
                data[pos] = value;
            }
            // 16
            uint16_t get16(uint8_t* data, uint32_t pos) override {
                return (data[pos + 0] << 8) | (data[pos + 1] << 0);
            }
            void     put16(uint8_t* data, uint32_t pos, uint16_t value) override {
                data[pos + 0] = (uint8_t)(value >> 8);
                data[pos + 1] = (uint8_t)(value >> 0);
            }
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
        } impl;

        static ByteBuffer getInstance(uint8_t* data, uint32_t size) {
            return ByteBuffer(impl, data, size);
        }
    };
    struct Net {
        static inline struct : public Impl {
            const char* name() override {
                return "NET";
            }
            // 8
            uint8_t  get8(uint8_t* data, uint32_t pos) override {
                return data[pos];
            }
            void     put8(uint8_t* data, uint32_t pos, uint8_t value) override {
                data[pos] = value;
            }
            // 16
            uint16_t get16(uint8_t* data, uint32_t pos) override {
                return (data[pos + 0] << 8) | (data[pos + 1] << 0);
            }
            void     put16(uint8_t* data, uint32_t pos, uint16_t value) override {
                data[pos + 0] = (uint8_t)(value >> 8);
                data[pos + 1] = (uint8_t)(value >> 0);
            }
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
        } impl;
    
        static ByteBuffer getInstance(uint8_t* data, uint32_t size) {
            return ByteBuffer(impl, data, size);
        }
    };

    ByteBuffer range(uint32_t wordOffset, uint32_t wordSize) const;

    const char* name() const {
        return myImpl.name();
    }

    const uint8_t* data() const {
        return myData;
    }

    uint32_t byteSize() const {
        return myByteSize;
    }
    uint32_t size() const {
        return byteValueToWordValue(byteSize());
    }
    void bytePos(uint32_t bytePos) {
        checkBytePos(bytePos);
        myBytePos = bytePos;
    }
    uint32_t bytePos() const {
        return myBytePos;
    }
    void pos(uint32_t wordPos) {
        bytePos(wordValueToByteValue(wordPos));
    }
    uint32_t pos() const {
        return byteValueToWordValue(bytePos());
    }
    
    // getX
    uint8_t get8() {
        const uint32_t readSize = 1;

        checkByteRange(myBytePos, readSize);
        auto ret = get8(myBytePos);
        myBytePos += readSize;
        return ret;
    }
    uint16_t get16() {
        const uint32_t readSize = 2;

        checkByteRange(myBytePos, readSize);
        auto ret = get16(myBytePos);
        myBytePos += readSize;
        return ret;
    }
    uint32_t get32() {
        const uint32_t readSize = 4;

        checkByteRange(myBytePos, readSize);
        auto ret = get32(myBytePos);
        myBytePos += readSize;
        return ret;
    }

    // putX
    void put8(uint8_t value) {
        const uint32_t readSize = 1;

        checkByteRange(myBytePos, readSize);
        put8(myBytePos, value);
        myBytePos += readSize;
    }
    void put16(uint16_t value) {
        const uint32_t readSize = 2;

        checkByteRange(myBytePos, readSize);
        put16(myBytePos, value);
        myBytePos += readSize;
    }
    void put32(uint32_t value) {
        const uint32_t readSize = 2;

        checkByteRange(myBytePos, readSize);
        put32(myBytePos, value);
        myBytePos += readSize;
    }


    //
    // HasRead and read(...)
    //
    struct HasRead {
        virtual ByteBuffer& read(ByteBuffer& bb) = 0;
    };
    ByteBuffer& read() {
        return *this;
    }
    template <class Head, class... Tail>
    ByteBuffer& read(Head&& head, Tail&&... tail) {
        constexpr auto is_uint8_t  = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint8_t>::value;
        constexpr auto is_uint16_t = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint16_t>::value;
        constexpr auto is_uint32_t = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint32_t>::value;
        constexpr auto is_class    = std::is_class_v<std::remove_reference_t<Head>>;

    //		logger.info("read head  %s!  %d  |  %d  %d  |  %d", demangle(typeid(head).name()), sizeof(head), is_uint16_t, is_uint32_t, is_class);

        if constexpr (is_uint8_t || is_uint16_t || is_uint32_t) {
            read(head);
        } else {
            if constexpr (is_class) {
                constexpr auto has_read = std::is_base_of_v<HasRead, std::remove_reference_t<Head>>;
                if constexpr (has_read) {
                    head.read(*this);
                } else {
                    logger.error("Unexptected type  %s", demangle(typeid(head).name()));
                    ERROR()
                }
            } else {
                logger.error("Unexptected type  %s", demangle(typeid(head).name()));
                ERROR()
            }
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
    ByteBuffer& read(int value) = delete;


    //
    // HasWrite and write(...)
    //
    struct HasWrite {
        virtual ByteBuffer& write(ByteBuffer& bb) const = 0;
    };
    ByteBuffer& write() {
        return *this;
    }
    template <class Head, class... Tail>
    ByteBuffer& write(Head&& head, Tail&&... tail) {
        constexpr auto is_uint8_t  = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint8_t>::value;
        constexpr auto is_uint16_t = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint16_t>::value;
        constexpr auto is_uint32_t = std::is_same<std::remove_cv_t<std::remove_reference_t<Head>>, uint32_t>::value;
        constexpr auto is_class    = std::is_class_v<std::remove_reference_t<Head>>;

    //		logger.info("write head  %s!  %d  |  %d  %d  |  %d", demangle(typeid(head).name()), sizeof(head), is_uint16_t, is_uint32_t, is_class);

        if constexpr (is_uint8_t || is_uint16_t || is_uint32_t) {
            write(head);
        } else {
            if constexpr (is_class) {
                constexpr auto has_write = std::is_base_of_v<HasWrite, std::remove_reference_t<Head>>;
                if constexpr (has_write) {
                    head.write(*this);
                } else {
                    logger.error("Unexptected type  %s", demangle(typeid(head).name()));
                    ERROR()
                }
            } else {
                logger.error("Unexptected type  %s", demangle(typeid(head).name()));
                ERROR()
            }
        }
        return read(std::forward<Tail>(tail)...);
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
    ByteBuffer& write(int value) = delete;

};
