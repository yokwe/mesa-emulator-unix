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
// MesaBuffer.h
//

#pragma once

#include <memory>
#include <cstdint>
#include <type_traits>
#include <string>

#include "../util/Util.h"


class MesaBuffer {
protected:
    uint8_t*  myData;
    uint32_t  myByteSize;
    uint32_t  myBytePos;

public:
    static uint8_t highByte(uint16_t value) {
        return (uint8_t)(value >> 8);
    }
    static uint8_t lowByte(uint16_t value) {
        return (uint8_t)(value >> 0);
    }

    MesaBuffer() : myData(0), myByteSize(0), myBytePos(0) {}
    MesaBuffer(uint8_t* data, uint32_t size) : myData(data), myByteSize(size), myBytePos(0) {}

    MesaBuffer range(uint32_t wordOffset, uint32_t WordSize);

    uint32_t byteSize() {
        return myByteSize;
    }
    uint32_t size() {
        return (byteSize() + 1) / 2;
    }
    void bytePos(uint32_t newValue);
    uint32_t bytePos() {
        return myBytePos;
    }
    void pos(uint32_t wordPos) {
        bytePos(wordPos * 2);
    }
    uint32_t pos() {
        return (bytePos() + 1) / 2;
    }

    uint8_t  get8();
    uint16_t get16();
    uint32_t get32();

    struct HasRead {
        virtual MesaBuffer& read(MesaBuffer& bb) = 0;
    };
    MesaBuffer& read() {
        return *this;
    }
    template <class Head, class... Tail>
    MesaBuffer& read(Head&& head, Tail&&... tail) {
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

    MesaBuffer& read(uint8_t& value) {
        value = get8();
        return *this;
    }
    MesaBuffer& read(uint16_t& value) {
        value = get16();
        return *this;
    }
    MesaBuffer& read(uint32_t& value) {
        value = get32();
        return *this;
    }
};

class MesaBufferArray : public MesaBuffer {
protected:
    std::shared_ptr<uint8_t> myPointer;

public:
    MesaBufferArray(std::vector<uint8_t>& vector) : MesaBuffer(new uint8_t[vector.size()], vector.size()), myPointer(std::shared_ptr<uint8_t>(myData)) {
        std::copy(vector.begin(), vector.end(), myData); // copy vector to myData
    }
};

class MesaBufferFile : public MesaBufferArray {
    std::string myPath;

public:
    static MesaBufferFile getInstance(const std::string& path);

    MesaBufferFile(const std::string& path, std::vector<uint8_t>& vector) : MesaBufferArray(vector), myPath(path) {}

    const std::string& path() {
        return myPath;
    }
};

