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

//#include "../util/Util.h"


class ByteBuffer {
    static const int bytesPerWord = 2;

protected:
    uint8_t*  myData;
    uint32_t  myByteSize;
    
    uint32_t  myBytePos;

    uint32_t byteValueToWordValue(uint32_t byteValue) {
        return (byteValue + bytesPerWord - 1) / bytesPerWord;
    }
    uint32_t wordValueToByteValue(uint32_t wordValue) {
        return wordValue * bytesPerWord;
    }

    void checkByteRange(uint32_t bytePos, uint32_t readSize) {
        checkBytePos(bytePos);
        checkBytePos(bytePos + readSize - 1);
    }
    void checkBytePos(uint32_t pos);

    uint8_t get8(uint32_t bytePos) {
        return myData[bytePos];
    }
    uint16_t get16(uint32_t bytePos) {
        return (myData[bytePos + 0] << 8) | (myData[bytePos + 1] << 0);
    }

    // netowrk order for 32bit value  high half and low half
    uint32_t get32Network(uint32_t bytePos) {
        return (myData[bytePos + 0] << 24) | (myData[bytePos + 1] << 16) | (myData[bytePos + 2] << 8) | (myData[bytePos + 3] << 0);
    }
    // mesa order for 32bit value  low half and high half
    uint32_t get32Mesa(uint32_t bytePos) {
        return (myData[bytePos + 0] << 8) | (myData[bytePos + 1] << 0) | (myData[bytePos + 2] << 24) | (myData[bytePos + 3] << 16);
    }

public:
    static uint8_t highByte(uint16_t value) {
        return (uint8_t)(value >> 8);
    }
    static uint8_t lowByte(uint16_t value) {
        return (uint8_t)(value >> 0);
    }

    ByteBuffer(uint8_t* data, const uint32_t byteSize) : myData(data), myByteSize(byteSize), myBytePos(0) {}
    virtual ~ByteBuffer() = default;

    const uint8_t* data() {
        return myData;
    }

    uint32_t byteSize() {
        return myByteSize;
    }
    uint32_t size() {
        return byteValueToWordValue(byteSize());
    }
    void bytePos(uint32_t bytePos) {
        checkBytePos(bytePos);
        myBytePos = bytePos;
    }
    uint32_t bytePos() {
        return myBytePos;
    }
    void pos(uint32_t wordPos) {
        bytePos(wordValueToByteValue(wordPos));
    }
    uint32_t pos() {
        return byteValueToWordValue(bytePos());
    }
    
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
    virtual uint32_t get32() = 0;

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
};

class MesaByteBuffer: public ByteBuffer {
public:
    MesaByteBuffer(uint8_t* data, const uint32_t size) : ByteBuffer(data, size) {}

    MesaByteBuffer range(uint32_t wordOffset, uint32_t wordSize) {
        auto bytePos  = wordValueToByteValue(wordOffset);
        auto readSize = wordValueToByteValue(wordSize);
        if (myByteSize < (bytePos + readSize)) {
            // fix readSize
            logger.info("unexpected value  readSize   %d   myByteSize  %d", readSize, myByteSize);
            readSize = myByteSize - bytePos;
        }
        // sanity check
        checkByteRange(bytePos, readSize);

        return MesaByteBuffer(myData + bytePos, readSize);
    }

    uint32_t get32() override {
        const uint32_t readSize = 4;

        // sanity check
        checkByteRange(myBytePos, readSize);

        auto ret = ByteBuffer::get32Mesa(myBytePos);
        myBytePos += readSize;
        return ret; 
    }
};


class NetworkByteBuffer: public ByteBuffer {
public:
    NetworkByteBuffer(uint8_t* data, const uint32_t byteSize) : ByteBuffer(data, byteSize) {}

    NetworkByteBuffer range(uint32_t wordOffset, uint32_t wordSize) {
        auto bytePos  = wordValueToByteValue(wordOffset);
        auto readSize = wordValueToByteValue(wordSize);
        if (myByteSize < (bytePos + readSize)) {
            // fix readSize
            logger.info("unexpected value  readSize   %d   myByteSize  %d", readSize, myByteSize);
            readSize = myByteSize - bytePos;
        }
        // sanity check
        checkByteRange(bytePos, readSize);
        
        return NetworkByteBuffer(myData + bytePos, readSize);
    }

    uint32_t get32() override {
        const uint32_t readSize = 4;

        // sanity check
        checkByteRange(myBytePos, myBytePos + readSize);

        auto ret = ByteBuffer::get32Network(myBytePos);
        myBytePos += readSize;
        return ret; 
    }
};
