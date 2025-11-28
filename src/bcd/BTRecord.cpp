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
// BTRecord.cpp
//

#include <string>
#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Symbols.h"
#include "SymbolsIndex.h"
#include "BTRecord.h"

#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

//
// BTIndex
//
//BTIndex: TYPE = Base RELATIVE POINTER [0..Limit) TO BodyRecord;
//BTNull: BTIndex = LAST[BTIndex];
std::string BTIndex::toString() const {
    if (isNull()) return std_sprintf("%s-NULL", prefix);
    else return Index::toString();
}

//
// BodyLink
//
std::string BodyLink::toString(Which value) {
    static std::map<Which, std::string> map {
        ENUM_VALUE(Which, SIBLING)
        ENUM_VALUE(Which, PARENT)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
ByteBuffer& BodyLink::read(ByteBuffer& bb) {
    uint16_t u0;

    bb.read(u0);

	// NOTICE
	//   BodyLink: TYPE = RECORD [which(0:0..0): {sibling(0), parent(1)}, index(0:1..14): BTIndex];
	//   Size of BodyLink is 15 bits.
	//   So need to shift 1 bit before process u0.
    u0 <<= 1;
    which = (Which)bitField(u0, 0);
    index.index(bitField(u0, 1, 14));

    return bb;
}
std::string BodyLink::toString() const {
    return std_sprintf("[%s %s]", toString(which), index.toString());
}

//
// BodyInfo
//
std::string BodyInfo::toString(Mark value) {
    static std::map<Mark, std::string> map {
        ENUM_VALUE(Mark, INTERNAL)
        ENUM_VALUE(Mark, EXTERNAL)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
void BodyInfo::INTERNAL::read(uint16_t u0, ByteBuffer& bb) {
    bodyTree.index(bitField(u0, 1, 15));
    bb.read(thread, frameSize);
}
std::string BodyInfo::INTERNAL::toString() const {
    return std_sprintf("[%s %s %d]", bodyTree.Index::toString(), thread.Index::toString(), frameSize);
}
void BodyInfo::EXTERNAL::read(uint16_t u0, ByteBuffer& bb) {
    bytes = bitField(u0, 1, 15);
    bb.read(startIndex, indexLength);
}
std::string BodyInfo::EXTERNAL::toString() const {
    return std_sprintf("[%d %d %d]", bytes, startIndex, indexLength);
}
ByteBuffer& BodyInfo::read(ByteBuffer& bb) {
    uint16_t u0;
    bb.read(u0);
    mark = (Mark)bitField(u0, 0);
    switch(mark) {
    case Mark::INTERNAL:
    {
        INTERNAL value;
        value.read(u0, bb);
        info = value;
    }
        break;
    case Mark::EXTERNAL:
    {
        EXTERNAL value;
        value.read(u0, bb);
        info = value;
    }
        break;
    default:
        ERROR()
    }
    return bb;
}
std::string BodyInfo::toString() const {
    std::string variantString = ::toString(info);
    return std_sprintf("[%s  %s]", toString(mark), variantString);
}

//
// BTRecord = BodyRecord
//
std::string BTRecord::toString(Kind value) {
    static std::map<Kind, std::string> map {
        ENUM_VALUE(Kind, CALLABLE)
        ENUM_VALUE(Kind, OTHER)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

//
// BTRecord::CALLABLE::OUTER
//
void BTRecord::CALLABLE::OUTER::read(uint16_t u11, ByteBuffer& bb) {
    (void)u11;
    (void)bb;
}

//
// BTRecord::CALLABLE::INNER
//
void BTRecord::CALLABLE::INNER::read(uint16_t u11, ByteBuffer& bb) {
    frameOffset = bitField(u11, 2, 15);
    (void)bb;
    // NOTICE
    //   below get16() is needed practically
    bb.get16(); // 12
}

//
// BTRecord::CALLABLE::CATCH
//
void BTRecord::CALLABLE::CATCH::read(uint16_t u11, ByteBuffer& bb) {
    (void)u11;
    bb.read(index); // 12
}
//
// BTRecord::CALLABLE
//
std::string BTRecord::CALLABLE::toString(Nesting value) {
    static std::map<Nesting, std::string> map {
        ENUM_VALUE(Nesting, OUTER)
        ENUM_VALUE(Nesting, INNER)
        ENUM_VALUE(Nesting, CATCH)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
void BTRecord::CALLABLE::read(uint16_t u8, ByteBuffer& bb) {
    uint16_t u9, u10, u11;
    bb.read(u9, u10, u11);

    inline_    = bitField(u8, 1);
    id.index(bitField(u8, 2, 15));
    ioType.index(bitField(u9, 0, 13));
    monitored  = bitField(u9, 14);
    noXfers    = bitField(u9, 15);
    resident   = bitField(u10, 0);
    entry      = bitField(u10, 1);
    internal   = bitField(u10, 2);
    entryIndex = bitField(u10, 3, 10);
    hints      = bitField(u10, 11, 15);
    nesting    = (Nesting)bitField(u11, 0);

    switch(nesting) {
    case Nesting::OUTER:
    {
        OUTER value;
        value.read(u11, bb);
        closure = value;
    }
        break;
    case Nesting::INNER:
    {
        INNER value;
        value.read(u11, bb);
        closure = value;
    }
        break;
    case Nesting::CATCH:
    {
        CATCH value;
        value.read(u11, bb);
        closure = value;
    }
        break;
    default:
        ERROR()
    }
}
std::string BTRecord::CALLABLE::toString() const {
    std::string closureString = ::toString(closure);
    std::string flags = std_sprintf("[%s%s%s%s%s%s %X]",
        inline_   ? "I" : "_",
        monitored ? "M" : "_",
        noXfers   ? "N" : "_",
        resident  ? "R" : "_",
        entry     ? "E" : "_",
        internal  ? "I" : "_",
        hints);
    return std_sprintf("[%s  %s  %d  %s  %s  %s]",
        id.Index::toString(),
        ioType.Index::toString(),
        entryIndex,
        flags,
        toString(nesting),
        closureString);
}

//
// BTRecord::OTHER
//
void BTRecord::OTHER::read(uint16_t u8, ByteBuffer& bb) {
    relOffset = bitField(u8, 1, 15);
    (void)bb;
}
std::string BTRecord::OTHER::toString() const {
    return std_sprintf("[%d]", relOffset);
}

//
// BTRecord
//
ByteBuffer& BTRecord::read(ByteBuffer& bb) {
    uint16_t u3, u8;
    bb.read(link, firstSon, type, u3, sourceIndex, info, u8);

    localCtx.index(bitField(u3,0, 10));
    level = (ContextLevel)bitField(u3, 11, 15);
    kind = (Kind)bitField(u8, 0);
    switch(kind) {
    case Kind::CALLABLE:
    {
        CALLABLE value;
        value.read(u8, bb);
        extension = value;
    }
        break;
    case Kind::OTHER:
    {
        OTHER value;
        value.read(u8, bb);
        extension = value;
    }
        break;
    default:
        ERROR()
    }
    return bb;
}
std::string BTRecord::toString() const {
    std::string extensionString = ::toString(extension);
    return std_sprintf("[%s  %s  %s  %s  %s  %d  %s  %s  %s]",
        link.toString(),
        firstSon.toString(),
        type.Index::toString(),
        localCtx.Index::toString(),
        ::toString(level),
        sourceIndex,
        info.toString(),
        toString(kind),
        extensionString);
}
