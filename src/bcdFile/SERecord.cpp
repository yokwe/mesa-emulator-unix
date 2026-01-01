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

#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "MesaByteBuffer.h"

#include "SERecord.h"


#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},


//
// SERecord::ID
//
std::string SERecord::ID::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_VALUE(Tag, TERMINAL)
        ENUM_VALUE(Tag, SEQUENTIAL)
        ENUM_VALUE(Tag, LINKED)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

void SERecord::ID::read(uint16_t u0, MesaByteBuffer& bb) {
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
    tag   = (Tag)bitField(u4, 14, 15);
    switch(tag) {
        case Tag::TERMINAL:
            variant = TERMINAL{};
            break;
        case Tag::SEQUENTIAL:
            variant = SEQUENTIAL{};
            break;
        case Tag::LINKED:
        {
            LINKED linked;
            bb.read(linked);
            variant = linked;
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

    std::string ctxLinkString = ::toString(variant);

    return
        std_sprintf("[%s  [%s]  %-8s  %5d  %5d  %-24s  %s  %s]",
            idCtx.toString(),
            flags,
            idType.toString(),
            idInfo,
            idValue,
            hash.toValue(),
            toString(tag),
            ctxLinkString);
}

SERecord::ID::LINKED SERecord::ID::toLINKED() const {
    if (tag != Tag::LINKED) ERROR()
    return std::get<SERecord::ID::LINKED>(variant);
}



//
// SERecord::CONS::BASIC
//
void SERecord::CONS::BASIC::read(uint16_t u0, MesaByteBuffer& bb) {
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
void SERecord::CONS::ENUMERATED::read(uint16_t u0, MesaByteBuffer& bb) {
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
        flags, valueCtx.toString(), nValues);
}


//
// SERecord::CONS::RECORD
//
std::string SERecord::CONS::RECORD::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_VALUE(Tag, NOT_LINKED)
        ENUM_VALUE(Tag, LINKED)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
void SERecord::CONS::RECORD::read(uint16_t u0, MesaByteBuffer& bb) {
    uint16_t u2;

    hints      = bitField(u0, 8, 15);
    bb.read(length, u2);
    argument   = bitField(u2,  0);
    monitored  = bitField(u2,  1);
    machindDep = bitField(u2,  2);
    painted    = bitField(u2,  3);
    fieldCtx.index(bitField(u2, 4, 14));
    tag    = (Tag)bitField(u2, 15);
    switch (tag) {
    case Tag::NOT_LINKED:
        variant = NOT_LINKED{};
        break;
    case Tag::LINKED:
    {
        LINKED linked;
        bb.read(linked);
        variant = linked;
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

    std::string linkPartString = ::toString(variant);
    return std_sprintf("[[%s] %d  %s  %s]",
        flags, length, fieldCtx.toString(), linkPartString);
}

//
// SERecord::CONS::RECORD::LINKED
//
MesaByteBuffer& SERecord::CONS::RECORD::LINKED::read(MesaByteBuffer& bb) {
    bb.read(linkType);
    return bb;
}
std::string SERecord::CONS::RECORD::LINKED::toString() const {
    return std_sprintf("[%s]", linkType.toString());
}


//
// SERecord::CONS::REF
//
void SERecord::CONS::REF::read(uint16_t u0, MesaByteBuffer& bb) {
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
        flags, refType.toString());
}


//
// SERecord::CONS::ARRAY
//
void SERecord::CONS::ARRAY::read(uint16_t u0, MesaByteBuffer& bb) {
    packed = bitField(u0, 8, 15);
    bb.read(indexType, componentType);
}
std::string SERecord::CONS::ARRAY::toString() const {
    return std_sprintf("[%d  %s  %s]", packed, indexType.toString(), componentType.toString());
}


//
// SERecord::CONS::ARRAYDESC
//
void SERecord::CONS::ARRAYDESC::read(uint16_t u0, MesaByteBuffer& bb) {
    var = bitField(u0, 8);
    readOnly = bitField(u0, 9, 15);
    bb.read(describedType);
}
std::string SERecord::CONS::ARRAYDESC::toString() const {
    std::string flags = std_sprintf("%s%s",
        var      ? "V" : "_",
        readOnly ? "R" : "_"
    );
    return std_sprintf("[[%s]  %s]", flags, describedType.toString());
}


//
// SERecord::CONS::TRANSFER
//
void SERecord::CONS::TRANSFER::read(uint16_t u0, MesaByteBuffer& bb) {
    safe = bitField(u0, 8);
    mode = (TransferMode)bitField(u0, 9, 15);
    bb.read(typeIn, typeOut);
}
std::string SERecord::CONS::TRANSFER::toString() const {
    return std_sprintf("[%s  %s  %s  %s]",
        safe ? "S" : "_", ::toString(mode), typeIn.toString(), typeOut.toString());
}


//
// SERecord::CONS::DEFINITION
//
void SERecord::CONS::DEFINITION::read(uint16_t u0, MesaByteBuffer& bb) {
    named = bitField(u0, 8, 15);
    bb.read(defCtx);
}
std::string SERecord::CONS::DEFINITION::toString() const {
    return std_sprintf("[%s  %s]", named ? "N" : "_", defCtx.toString());
}


//
// SERecord::CONS::UNION
//
void SERecord::CONS::UNION::read(uint16_t u0, MesaByteBuffer& bb) {
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
        flags, caseCtx.toString(), tagSei.toString());
}


//
// SERecord::CONS::SEQUENCE
//
void SERecord::CONS::SEQUENCE::read(uint16_t u0, MesaByteBuffer& bb) {
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
        flags, tagSei.toString(), componentType.toString());
}

//
// SERecord::CONS::RELATIVE
//
void SERecord::CONS::RELATIVE::read(uint16_t u0, MesaByteBuffer& bb) {
    (void)u0;
    bb.read(baseType, offsetType, resultType);
}
std::string SERecord::CONS::RELATIVE::toString() const {
    return std_sprintf("[%s  %s  %s]",
        baseType.toString(), offsetType.toString(), resultType.toString());
}


//
// SERecord::CONS::SUBRANGE
//
void SERecord::CONS::SUBRANGE::read(uint16_t u0, MesaByteBuffer& bb) {
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
        flags, rangeType.toString(), origin, range);
}


//
// SERecord::CONS::LONG
//
void SERecord::CONS::LONG::read(uint16_t u0, MesaByteBuffer& bb) {
    (void)u0;
    bb.read(rangeType);
}
std::string SERecord::CONS::LONG::toString() const {
    return std_sprintf("[%s]", rangeType.toString());
}


//
// SERecord::CONS::REAL
//
void SERecord::CONS::REAL::read(uint16_t u0, MesaByteBuffer& bb) {
    (void)u0;
    bb.read(rangeType);
}
std::string SERecord::CONS::REAL::toString() const {
    return std_sprintf("[%s]", rangeType.toString());
}


//
// SERecord::CONS::OPAQUE
//
void SERecord::CONS::OPAQUE::read(uint16_t u0, MesaByteBuffer& bb) {
    lengthKnown = bitField(u0, 8, 15);
    bb.read(length, id);
}
std::string SERecord::CONS::OPAQUE::toString() const {
    return std_sprintf("[%d  %d  %s]",
        lengthKnown, length, id.toString());
}


//
// SERecord::CONS::ZONE
//
void SERecord::CONS::ZONE::read(uint16_t u0, MesaByteBuffer& bb) {
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
#define CASE_BODY(typeName) { typeName value; value.read(u0, bb); variant = value; }
void SERecord::CONS::read(uint16_t u0, MesaByteBuffer& bb) {
    tag = (TypeClass)bitField(u0, 3, 7);
    switch(tag) {
        
    case TypeClass::MODE:
        variant = MODE{};
        break;
    case TypeClass::BASIC:
        CASE_BODY(BASIC)
        break;
    case TypeClass::ENUMERATED:
        CASE_BODY(ENUMERATED)
        break;
    case TypeClass::RECORD:
        CASE_BODY(RECORD)
        break;
    case TypeClass::REF:
        CASE_BODY(REF)
        break;
    case TypeClass::ARRAY:
        CASE_BODY(ARRAY)
        break;
    case TypeClass::ARRAYDESC:
        CASE_BODY(ARRAYDESC)
        break;
    case TypeClass::TRANSFER:
        CASE_BODY(TRANSFER)
        break;
    case TypeClass::DEFINITION:
        CASE_BODY(DEFINITION)
        break;
    case TypeClass::UNION:
        CASE_BODY(UNION)
        break;
    case TypeClass::SEQUENCE:
        CASE_BODY(SEQUENCE)
        break;
    case TypeClass::RELATIVE:
        CASE_BODY(RELATIVE)
        break;
    case TypeClass::SUBRANGE:
        CASE_BODY(SUBRANGE)
        break;
    case TypeClass::LONG:
        CASE_BODY(LONG)
        break;
    case TypeClass::REAL:
        CASE_BODY(REAL)
        break;
    case TypeClass::OPAQUE:
        CASE_BODY(OPAQUE)
        break;
    case TypeClass::ZONE:
        CASE_BODY(ZONE)
        break;
    case TypeClass::ANY:
        variant = ANY{};
        break;
    case TypeClass::NIL:
        variant = NIL{};
        break;
    case TypeClass::BITS:
        CASE_BODY(BITS)
        break;
    default:
        ERROR()
    }
}
std::string SERecord::CONS::toString() const {
    return std_sprintf("[%s  %s]", ::toString(tag), ::toString(variant));
}


//
// SERecord
//
std::string SERecord::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_VALUE(Tag, ID)
        ENUM_VALUE(Tag, CONS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
MesaByteBuffer& SERecord::read(MesaByteBuffer& bb) {
    uint16_t u0;

    bb.read(u0);

    tag = (Tag)bitField(u0, 2);
    switch(tag) {
    case Tag::ID:
    {
        ID id;
        id.read(u0, bb);
        variant = id;
    }
        break;
    case Tag::CONS:
    {
        CONS cons;
        cons.read(u0, bb);
        variant = cons;
    }
        break;
    default:
        ERROR()
    }

    return bb;
}
std::string SERecord::toString() const {
    return std_sprintf("[%s  %s]", toString(tag), ::toString(variant));
}

SERecord::ID   SERecord::toID() const {
    if (tag != Tag::ID) ERROR()
    return std::get<SERecord::ID>(variant);
}
SERecord::CONS SERecord::toCONS() const {
    if (tag != Tag::CONS) ERROR()
    return std::get<SERecord::CONS>(variant);
}
