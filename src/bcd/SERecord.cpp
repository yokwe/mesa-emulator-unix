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
// SERecord.cpp
//

#include <string>
#include <variant>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "SERecord.h"


#undef MAP_ENTRY
#define MAP_ENTRY(value) {T::value, #value},

//
// SEIndex
//
std::string SEIndex::toString() const {
    if (isNull()) return std_sprintf("%s-NULL", prefix);
    return value()->toString();
}


//
// SERecord
//

#undef ENUM_CLASS
#define ENUM_CLASS SERecord::Type
std::string SERecord::toString(ENUM_CLASS value) { // 01
    using T = ENUM_CLASS;
    static std::map<T, std::string> map {
        MAP_ENTRY(ID)
        MAP_ENTRY(CONS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}


//
// SERecord::ID
//

#undef ENUM_CLASS
#define ENUM_CLASS SERecord::ID::Type
std::string SERecord::ID::toString(ENUM_CLASS value) { // 02
    using T = ENUM_CLASS;
    static std::map<T, std::string> map {
        MAP_ENTRY(TERMINAL)
        MAP_ENTRY(SEQUENTIAL)
        MAP_ENTRY(LINKED)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

void SERecord::ID::read(uint16_t u0, ByteBuffer& bb) {
    uint16_t u1, u4;

    bb.read(u1, idInfo, idValue, u4);

    extended  = bitField(u0, 3);
    public_   = bitField(u0, 4);
    idCtx.index(bitField(u0, 5, 15));
    immutable = bitField(u1, 0);
    constant  = bitField(u1, 1);
    idType.index(bitField(u1, 2, 15));
    hash.index(bitField(u4, 0, 12));
    linkSpace = bitField(u4, 13);
    linkTag   = (Type)bitField(u4, 14, 15);
    switch(linkTag) {
        case Type::TERMINAL:
            ctxLink = TERMINAL{};
            break;
        case Type::SEQUENTIAL:
            ctxLink = SEQUENTIAL{};
            break;
        case Type::LINKED:
        {
            LINKED linked;
            bb.read(linked);
            ctxLink = linked;
        }
            break;
        default:
            ERROR()
    }
}

std::string SERecord::ID::toString() const {
    std::string flags = std_sprintf("%s%s%s%s%s",
        extended ? "X" : "",
        public_ ? "P" : "",
        immutable ? "I" : "",
        constant ? "C" : "",
        linkSpace ? "L" : ""
    );

    std::string ctxLinkString = ::toString(ctxLink);

    return
        std_sprintf("[%s  [%s]  %s  %5d  %5d  %s  %s  %s]",
            idCtx.Index::toString(),
            flags,
            idType.toString(),
            idInfo,
            idValue,
            hash.toValue(),
            toString(linkTag),
            ctxLinkString);
}


//
// SERecord::CONS
//

#undef ENUM_CLASS
#define ENUM_CLASS SERecord::CONS::Type
std::string SERecord::CONS::toString(ENUM_CLASS value) {
    using T = ENUM_CLASS;
    static std::map<T, std::string> map {
        MAP_ENTRY(MODE)
        MAP_ENTRY(BASIC)
        MAP_ENTRY(ENUMERATED)
        MAP_ENTRY(RECORD)
        MAP_ENTRY(REF)
        //
        MAP_ENTRY(ARRAY)
        MAP_ENTRY(ARRAYDESC)
        MAP_ENTRY(TRANSFER)
        MAP_ENTRY(DEFINITION)
        MAP_ENTRY(UNION)
        //
        MAP_ENTRY(SEQUENCE)
        MAP_ENTRY(RELATIVE)
        MAP_ENTRY(SUBRANGE)
        MAP_ENTRY(LONG)
        MAP_ENTRY(REAL)
        //
        MAP_ENTRY(OPAQUE)
        MAP_ENTRY(ZONE)
        MAP_ENTRY(ANY)
        MAP_ENTRY(NIL)
        MAP_ENTRY(BITS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

