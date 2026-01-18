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
// LTRecord.cpp
//

#include <cstdint>
#include <string>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/ByteBuffer.h"

#include "LTRecord.h"

#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

//
// LTRecord
//
std::string LTRecord::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_VALUE(Tag, SHORT)
        ENUM_VALUE(Tag, LONG)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

ByteBuffer& LTRecord::LONG::read(ByteBuffer& bb) {
    bb.read(codeIndex, length);
    for(int i = 0; i < length; i++) {
        value.push_back(bb.get16());
    }
    return bb;
}
std::string LTRecord::LONG::toString() const {
    std::string string;
    for(int i = 0; i < length; i++) {
        string += std_sprintf("%04X", value.at(i));
    }
    return std_sprintf("[%d, (%d)%s]", codeIndex, length, string);
}
ByteBuffer& LTRecord::read(ByteBuffer& bb) {
    uint16_t u0;
    bb.read(u0);
    tag = (Tag)bitField(u0, 13, 15);

    SHORT shortValue;
    LONG  longValue;
    switch(tag) {
    case Tag::SHORT:
        bb.read(shortValue);
        variant = shortValue;
        break;
    case Tag::LONG:
        bb.read(longValue);
        variant = longValue;
        break;
    default:
        ERROR()
    }

    return bb;
}
std::string LTRecord::toString() const {
    std::string variantString = ::toString(variant);
    return std_sprintf("[%s %s]",
        toString(tag),
        variantString);
}


//
// LitRecord
//
std::string LitRecord::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_VALUE(Tag, WORD)
        ENUM_VALUE(Tag, STRING)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
// NORICE
//   size of LitRecod is 14 bits
//   needs to shift 2 bit for 16 bit aligned data
void LitRecord::read(uint16_t u0) {
    u0 <<= 2;
    tag = (Tag)bitField(u0, 0);
    WORD   wordValue;
    STRING stringValue;
    switch(tag) {
    case Tag::WORD:
        wordValue.read(u0);
        variant = wordValue;
        break;
    case Tag::STRING:
        stringValue.read(u0);
        variant = stringValue;
        break;
    default:
        ERROR()
    }
}
std::string LitRecord::toString() const {
    std::string varintString = ::toString(variant);
    return std_sprintf("[%s  %s]", toString(tag), varintString);
}
