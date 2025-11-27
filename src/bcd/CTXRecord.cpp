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
// CTXRecord.cpp
//

#include <string>
#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Symbols.h"
#include "CTXRecord.h"

#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

std::string CTXRecord::toString(Closure value) {
    static std::map<Closure, std::string> map {
        ENUM_VALUE(Closure, NONE)
        ENUM_VALUE(Closure, UNIT)
        ENUM_VALUE(Closure, RC)
        ENUM_VALUE(Closure, FULL)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string CTXRecord::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, SIMPLE)
        ENUM_VALUE(Type, INCLUDED)
        ENUM_VALUE(Type, IMPORTED)
        ENUM_VALUE(Type, NIL)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}


//
// CTXIndex
//
std::string CTXIndex::toString() const {
    if (isNull()) return std_sprintf("%s-NULL", prefix);
    return value()->toString();
}


//
// CTXRecord::simple
//
CTXRecord::simple::simple(uint16_t u1) {
    ctxNew.index(bitField(u1, 5, 15));
}
std::string CTXRecord::simple::toString() const {
    return ctxNew.toString();
}

//
// CTXRecord::included
//
CTXRecord::included::included(uint16_t u1, ByteBuffer& bb) {
    uint16_t u2;
    uint16_t u3;

    bb.read(u2, u3);
    chain.index(bitField(u1, 5, 15));
    copied     = (Closure)bitField(u2, 0,  1);
    module.index(bitField(u2, 2, 15));
    map.index(bitField(u3, 0, 10));
    closed     = bitField(u3, 11);
    complete   = bitField(u3, 12);
    restricted = bitField(u3, 13);
    reset      = bitField(u3, 14, 15);
}
std::string CTXRecord::included::toString() const {
    return std_sprintf("[%s  %s  %s %s%s%s%s]",
        chain.Index::toString(),
        CTXRecord::toString(copied),
        map.toString(),
        closed ? " closed" : "",
        complete ? " complete" : "",
        restricted ? " restricted" : "",
        reset ? " reset" : "");
}

//
// CTXRecord::imported
//
CTXRecord::imported::imported(uint16_t u1) {
    includeIndex.index(bitField(u1, 5, 15));
}
std::string CTXRecord::imported::toString() const {
    return includeIndex.Index::toString();
}

//
// CTXReord
//
ByteBuffer& CTXRecord::read(ByteBuffer& bb) {
    uint16_t u0, u1;
    bb.read(u0, u1);

    mark        = bitField(u0, 0);
    varUpdated  = bitField(u0, 1);
    seList.index(bitField(u0, 2, 15));
//    seList      = bitField(u0, 2, 15);
    level       = (ContextLevel)bitField(u1, 0,  2);
    ctxType     = (Type)bitField(u1, 3, 4);
    switch(ctxType) {
    case Type::SIMPLE:
        extension = simple(u1);
        break;
    case Type::INCLUDED:
        extension = included(u1, bb);
        break;
    case Type::IMPORTED:
        extension = imported(u1);
        break;
    case Type::NIL:
        extension = nil();
        break;
    default:
        ERROR();
    }

    return bb;
}
std::string CTXRecord::toString() const {
    std::string extString = std::visit([](auto& x) { return x.toString(); }, extension);
    return std_sprintf("[%s%s  %5d  %s  %s  %s]",
        mark ? "M" : " ", varUpdated ? "V" : " ",
        seList.Index::toString(), ::toString(level), toString(ctxType), extString);
}
