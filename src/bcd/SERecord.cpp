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

#include "Index.h"

#include "SERecord.h"


#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

//
// SEIndex
//
std::string SEIndex::toString() const {
    if (isNull()) return std_sprintf("%s-NULL", prefix);
    return value()->toString();
}


//
// SERecord::ID
//
std::string SERecord::ID::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, TERMINAL)
        ENUM_VALUE(Type, SEQUENTIAL)
        ENUM_VALUE(Type, LINKED)
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
        extended  ? "E" : "_",
        public_   ? "P" : "_",
        immutable ? "I" : "_",
        constant  ? "C" : "_",
        linkSpace ? "L" : "_"
    );

    std::string ctxLinkString = ::toString(ctxLink);

    return
        std_sprintf("[%s  [%s]  %-8s  %5d  %5d  %s  %s  %s]",
            idCtx.Index::toString(),
            flags,
            idType.Index::toString(),
            idInfo,
            idValue,
            hash.toValue(),
            toString(linkTag),
            ctxLinkString);
}


//
// SERecord::CONS::BASIC
//
void SERecord::CONS::BASIC::read(uint16_t u0, ByteBuffer& bb) {
    ordered = bitField(u0, 8);
    code    = bitField(9, 15);
    bb.read(length);
}
std::string SERecord::CONS::BASIC::toString() const {
    return std_sprintf("[%d  %d  %d]", ordered, code, length);
}


//
// SERecord::CONS::ENUMERATED
//
void SERecord::CONS::ENUMERATED::read(uint16_t u0, ByteBuffer& bb) {
    ordered    = bitField(u0,  8);
    machineDep = bitField(u0,  9);
    unpainted  = bitField(u0, 10);
    sparse     = bitField(u0, 11, 15);
    bb.read(valueCtx, nValues);
}
std::string SERecord::CONS::ENUMERATED::toString() const {
    std::string flags = std_sprintf("%s%s%s%s",
        ordered    ? "O" : "_",
        machineDep ? "M" : "_",
        unpainted  ? "U" : "_",
        sparse     ? "S" : "_");
    return std_sprintf("[[%s]  %s  %d]",
        flags, valueCtx.Index::toString(), nValues);
}


//
// SERecord::CONS::RECORD
//
std::string SERecord::CONS::RECORD::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, NOT_LINKED)
        ENUM_VALUE(Type, LINKED)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
void SERecord::CONS::RECORD::read(uint16_t u0, ByteBuffer& bb) {
    uint16_t u2;

    hints      = bitField(u0, 8, 15);
    bb.read(length, u2);
    argument   = bitField(u2,  0);
    monitored  = bitField(u2,  1);
    machindDep = bitField(u2,  2);
    painted    = bitField(u2,  3);
    fieldCtx.index(bitField(u2, 4, 14));
    linkTag    = (Type)bitField(u2, 15);
    switch (linkTag) {
    case Type::NOT_LINKED:
        linkPart = NOT_LINKED{};
        break;
    case Type::LINKED:
    {
        LINKED linked;
        bb.read(linked);
        linkPart = linked;
    }
        break;
    default:
        ERROR()
    }
}
std::string SERecord::CONS::RECORD::toString() const {
    std::string flags = std_sprintf("%02X %s%s%s%s",
        hints,
        argument   ? "A" : "_",
        monitored  ? "m" : "_",
        machindDep ? "M" : "_",
        painted    ? "P" : "_");

    std::string linkPartString = ::toString(linkPart);
    return std_sprintf("[[%s] %d  %s  %s]",
        flags, length, fieldCtx.Index::toString(), linkPartString);
}

//
// SERecord::CONS::RECORD::LINKED
//
ByteBuffer& SERecord::CONS::RECORD::LINKED::read(ByteBuffer& bb) {
    bb.read(linkType);
    return bb;
}
std::string SERecord::CONS::RECORD::LINKED::toString() const {
    return std_sprintf("[%s]", linkType.Index::toString());
}


//
// SERecord::CONS::REF
//
void SERecord::CONS::REF::read(uint16_t u0, ByteBuffer& bb) {
    counted  = bitField(u0,  8);
    ordered  = bitField(u0,  9);
    readOnly = bitField(u0, 10);
    list     = bitField(u0, 11);
    var      = bitField(u0, 12);
    basing   = bitField(u0, 13, 15);
    bb.read(refType);
}
std::string SERecord::CONS::REF::toString() const {
    std::string flags = std_sprintf("%s%s%s%s%s",
        counted ? "C" : "_",
        ordered ? "O" : "_",
        readOnly ? "R" : "_",
        list ? "L" : "_",
        var ? "V" : "_",
        basing ? "B" : "_");
    return std_sprintf("[[%s]  %s]",
        flags, refType.Index::toString());
}


//
// SERecord::CONS::ARRAY
//
void SERecord::CONS::ARRAY::read(uint16_t u0, ByteBuffer& bb) {
    packed = bitField(u0, 8, 15);
    bb.read(indexType, componentType);
}
std::string SERecord::CONS::ARRAY::toString() const {
    return std_sprintf("[%d  %s  %s]", packed, indexType.Index::toString(), componentType.Index::toString());
}


//
// SERecord::CONS::ARRAYDESC
//
void SERecord::CONS::ARRAYDESC::read(uint16_t u0, ByteBuffer& bb) {
    var = bitField(u0, 8);
    readOnly = bitField(u0, 9, 15);
    bb.read(describedType);
}
std::string SERecord::CONS::ARRAYDESC::toString() const {
    std::string flags = std_sprintf("%s%s",
        var      ? "V" : "_",
        readOnly ? "R" : "_"
    );
    return std_sprintf("[[%s]  %s]", flags, describedType.Index::toString());
}


//
// SERecord::CONS::TRANSFER
//
std::string SERecord::CONS::TRANSFER::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, PROC)
        ENUM_VALUE(Type, PORT)
        ENUM_VALUE(Type, SIGNAL)
        ENUM_VALUE(Type, ERROR_)
        ENUM_VALUE(Type, PROCESS)
        ENUM_VALUE(Type, PROGRAM)
        ENUM_VALUE(Type, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
void SERecord::CONS::TRANSFER::read(uint16_t u0, ByteBuffer& bb) {
    safe = bitField(u0, 8);
    mode = (Type)bitField(u0, 9, 15);
    bb.read(typeIn, typeOut);
}
std::string SERecord::CONS::TRANSFER::toString() const {
    return std_sprintf("[%s  %s  %s  %s]",
        safe ? "S" : "_", toString(mode), typeIn.Index::toString(), typeOut.Index::toString());
}


//
// SERecord::CONS::DEFINITION
//
void SERecord::CONS::DEFINITION::read(uint16_t u0, ByteBuffer& bb) {
    named = bitField(u0, 8, 15);
    bb.read(defCtx);
}
std::string SERecord::CONS::DEFINITION::toString() const {
    return std_sprintf("[%s  %s]", named ? "N" : "_", defCtx.Index::toString());
}


//
// SERecord::CONS::UNION
//
void SERecord::CONS::UNION::read(uint16_t u0, ByteBuffer& bb) {
    hints      = bitField(u0,  8, 11);
    overlaid   = bitField(u0, 12);
    controlled = bitField(u0, 13);
    machineDep = bitField(u0, 14, 15);
    bb.read(caseCtx, tagSei);
}
std::string SERecord::CONS::UNION::toString() const {
    std::string flags = std_sprintf("%02X %s%s%s",
        hints,
        overlaid   ? "O" : "_",
        controlled ? "C" : "_",
        machineDep ? "M" : "_"
    );
    return std_sprintf("[[%s]  %s  %s]",
        flags, caseCtx.Index::toString(), tagSei.Index::toString());
}


//
// SERecord::CONS::SEQUENCE
//
void SERecord::CONS::SEQUENCE::read(uint16_t u0, ByteBuffer& bb) {
    packed     = bitField(u0, 8);
    controlled = bitField(u0, 9);
    machindDep = bitField(u0, 10, 15);
    bb.read(tagSei, componentType);
}
std::string SERecord::CONS::SEQUENCE::toString() const {
    std::string flags = std_sprintf("%s%s%s",
        packed ? "P" : "_",
        controlled ? "C" : "_",
        machindDep ? "M" : "_");
    return std_sprintf("[[%s]  %s  %s]",
        flags, tagSei.Index::toString(), componentType.Index::toString());
}

//
// SERecord::CONS::RELATIVE
//
void SERecord::CONS::RELATIVE::read(uint16_t u0, ByteBuffer& bb) {
    (void)u0;
    bb.read(baseType, offsetType, resultType);
}
std::string SERecord::CONS::RELATIVE::toString() const {
    return std_sprintf("[%s  %s  %s]",
        baseType.Index::toString(), offsetType.Index::toString(), resultType.Index::toString());
}


//
// SERecord::CONS::SUBRANGE
//
void SERecord::CONS::SUBRANGE::read(uint16_t u0, ByteBuffer& bb) {
    filled = bitField(u0, 8);
    empty  = bitField(u0, 9, 15);
    uint16_t uorigin;
    bb.read(rangeType, uorigin, range);
    origin = (int16_t)uorigin;
}
std::string SERecord::CONS::SUBRANGE::toString() const {
    std::string flags = std_sprintf("%s%s",
        filled ? "F" : "_",
        empty  ? "E" : "_"
    );
    return std_sprintf("[[%s]  %s  %d  %d]",
        flags, rangeType.Index::toString(), origin, range);
}


//
// SERecord::CONS::LONG
//
void SERecord::CONS::LONG::read(uint16_t u0, ByteBuffer& bb) {
    (void)u0;
    bb.read(rangeType);
}
std::string SERecord::CONS::LONG::toString() const {
    return std_sprintf("[%s]", rangeType.Index::toString());
}


//
// SERecord::CONS::REAL
//
void SERecord::CONS::REAL::read(uint16_t u0, ByteBuffer& bb) {
    (void)u0;
    bb.read(rangeType);
}
std::string SERecord::CONS::REAL::toString() const {
    return std_sprintf("[%s]", rangeType.Index::toString());
}


//
// SERecord::CONS::OPAQUE
//
void SERecord::CONS::OPAQUE::read(uint16_t u0, ByteBuffer& bb) {
    lengthKnown = bitField(u0, 8, 15);
    bb.read(length, id);
}
std::string SERecord::CONS::OPAQUE::toString() const {
    return std_sprintf("[%d  %d  %s]",
        lengthKnown, length, id.Index::toString());
}


//
// SERecord::CONS::ZONE
//
void SERecord::CONS::ZONE::read(uint16_t u0, ByteBuffer& bb) {
    (void)bb;
    counted = bitField(u0, 8);
    mds     = bitField(u0, 9, 15);
}
std::string SERecord::CONS::ZONE::toString() const {
    return std_sprintf("[%s%s]",
        counted ? "C" : "_",
        mds     ? "M" : "_");
}


//
// SERecord::CONS
//
std::string SERecord::CONS::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, MODE)
        ENUM_VALUE(Type, BASIC)
        ENUM_VALUE(Type, ENUMERATED)
        ENUM_VALUE(Type, RECORD)
        ENUM_VALUE(Type, REF)
        //
        ENUM_VALUE(Type, ARRAY)
        ENUM_VALUE(Type, ARRAYDESC)
        ENUM_VALUE(Type, TRANSFER)
        ENUM_VALUE(Type, DEFINITION)
        ENUM_VALUE(Type, UNION)
        //
        ENUM_VALUE(Type, SEQUENCE)
        ENUM_VALUE(Type, RELATIVE)
        ENUM_VALUE(Type, SUBRANGE)
        ENUM_VALUE(Type, LONG) 
        ENUM_VALUE(Type, REAL)
        //
        ENUM_VALUE(Type, OPAQUE)
        ENUM_VALUE(Type, ZONE)
        ENUM_VALUE(Type, ANY)
        ENUM_VALUE(Type, NIL)
        ENUM_VALUE(Type, BITS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
#define CASE_BODY(typeName) { typeName value; value.read(u0, bb); typeInfo = value; }
void SERecord::CONS::read(uint16_t u0, ByteBuffer& bb) {
    typeTag = (Type)bitField(u0, 3, 7);
    switch(typeTag) {
        
    case Type::MODE:
        typeInfo = MODE{};
        break;
    case Type::BASIC:
        CASE_BODY(BASIC)
        break;
    case Type::ENUMERATED:
        CASE_BODY(ENUMERATED)
        break;
    case Type::RECORD:
        CASE_BODY(RECORD)
        break;
    case Type::REF:
        CASE_BODY(REF)
        break;
    case Type::ARRAY:
        CASE_BODY(ARRAY)
        break;
    case Type::ARRAYDESC:
        CASE_BODY(ARRAYDESC)
        break;
    case Type::TRANSFER:
        CASE_BODY(TRANSFER)
        break;
    case Type::DEFINITION:
        CASE_BODY(DEFINITION)
        break;
    case Type::UNION:
        CASE_BODY(UNION)
        break;
    case Type::SEQUENCE:
        CASE_BODY(SEQUENCE)
        break;
    case Type::RELATIVE:
        CASE_BODY(RELATIVE)
        break;
    case Type::SUBRANGE:
        CASE_BODY(SUBRANGE)
        break;
    case Type::LONG:
        CASE_BODY(LONG)
        break;
    case Type::REAL:
        CASE_BODY(REAL)
        break;
    case Type::OPAQUE:
        CASE_BODY(OPAQUE)
        break;
    case Type::ZONE:
        CASE_BODY(ZONE)
        break;
    case Type::ANY:
        typeInfo = ANY{};
        break;
    case Type::NIL:
        typeInfo = NIL{};
        break;
    case Type::BITS:
        CASE_BODY(BITS)
        break;
    default:
        ERROR()
    }
}
std::string SERecord::CONS::toString() const {
    return std_sprintf("[%s  %s]", toString(typeTag), ::toString(typeInfo));
}



//
// SERecord
//
std::string SERecord::toString(Type value) {
    static std::map<Type, std::string> map {
        ENUM_VALUE(Type, ID)
        ENUM_VALUE(Type, CONS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
ByteBuffer& SERecord::read(ByteBuffer& bb) {
    uint16_t u0;

    bb.read(u0);

    seTag = (Type)bitField(u0, 2);
    switch(seTag) {
    case Type::ID:
    {
        ID id;
        id.read(u0, bb);
        body = id;
    }
        break;
    case Type::CONS:
    {
        CONS cons;
        cons.read(u0, bb);
        body = cons;
    }
        break;
    default:
        ERROR()
    }

    return bb;
}
std::string SERecord::toString() const {
    return std_sprintf("[%s  %s]", toString(seTag), ::toString(body));
}
