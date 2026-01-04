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
// SERecord.h
//

#pragma once

#include <cstdint>
#include <string>

#include "../util/Util.h"

#include "MesaByteBuffer.h"

#include "Type.h"
#include "CTXIndex.h"
#include "SEIndex.h"
#include "HTIndex.h"

//SERecord: TYPE = RECORD [
//  mark3(0:0..0), mark4(0:1..1): BOOLEAN,
//  body(0:2..95): SELECT seTag(0:2..2): * FROM
//    id => [
//      extended(0:3..3): BOOLEAN,
//      public(0:4..4): BOOLEAN,
//      idCtx(0:5..15): CTXIndex,
//      immutable(1:0..0), constant(1:1..1): BOOLEAN,
//      idType(1:2..15): SEIndex,
//      idInfo(2:0..15): Unspec,
//      idValue(3:0..15): Unspec,
//      hash(4:0..12): HTIndex,
//      linkSpace(4:13..13): BOOLEAN,
//      ctxLink(4:14..31): SELECT linkTag(4:14..15): * FROM
//        terminal => [],
//        sequential => [],
//        linked => [link(5:0..15): ISEIndex]
//        ENDCASE],
//    cons => [
//      typeInfo(0:3..63): SELECT typeTag(0:3..7): TypeClass FROM
//        mode => [],
//        basic => [
//          ordered(0:8..8): BOOLEAN,
//          code(0:9..15): [0..16),
//          length(1:0..15): BitLength],
//        enumerated => [
//          ordered(0:8..8), machineDep(0:9..9): BOOLEAN,
//          unpainted(0:10..10): BOOLEAN,     -- un- for backward compatiblity
//          sparse(0:11..15): BOOLEAN,
//          valueCtx(1:0..15): CTXIndex,
//          nValues(2:0..15): CARDINAL],
//        record => [
//          hints(0:8..15): RECORD [
//            comparable(0:0..0), assignable(0:1..1): BOOLEAN,
//            unifield(0:2..2), variant(0:3..3), privateFields(0:4..4): BOOLEAN,
//            refField(0:5..5), default(0:6..6), voidable(0:7..7): BOOLEAN],
//          length(1:0..15): BitLength,
//          argument(2:0..0), monitored(2:1..1), machineDep(2:2..2): BOOLEAN,
//          painted(2:3..3): BOOLEAN,
//          fieldCtx(2:4..14): CTXIndex,
//          linkPart(2:15..31): SELECT linkTag(2:15..15): * FROM
//            notLinked => [],
//            linked => [linkType(3:0..15): SEIndex]
//            ENDCASE],
//        ref => [
//          counted(0:8..8), ordered(0:9..9), readOnly(0:10..10), list(0:11..11), var(0:12..12), basing(0:13..15): BOOLEAN,
//          refType(1:0..15): SEIndex],
//        array => [
//          packed(0:8..15): BOOLEAN,
//          indexType(1:0..15): SEIndex,
//          componentType(2:0..15): SEIndex],
//        arraydesc => [
//          var(0:8..8), readOnly(0::9..15): BOOLEAN,
//          describedType(1:0..15): SEIndex],
//        transfer => [
//          safe(0:8..8): BOOLEAN,
//          mode(0:9..15): TransferMode,
//          typeIn(1:0..15), typeOut(2:0..15): CSEIndex],
//        definition => [
//          named(0:8..15): BOOLEAN,
//          defCtx(1:0..15): CTXIndex],
//        union => [
//          hints(0:8..11): RECORD [
//            equalLengths(0:0..0): BOOLEAN,
//            refField(0:1..1), default(0:2..2), voidable(0:3..3): BOOLEAN],
//          overlaid(0:12..12), controlled(0:13..13), machineDep(0:14..15): BOOLEAN,
//          caseCtx(1:0..15): CTXIndex,
//          tagSei(2:0..15): ISEIndex],
//        sequence => [
//          packed(0:8..8): BOOLEAN,
//          controlled(0:9..9), machineDep(0:10..15): BOOLEAN,
//          tagSei(1:0..15): ISEIndex,
//          componentType(2:0..15): SEIndex],
//        relative => [
//          baseType(1:0..15): SEIndex,
//          offsetType(2:0..15): SEIndex,
//          resultType(3:0..15): SEIndex],
//        subrange => [
//          filled(0:8..8), empty(0:9..15): BOOLEAN,
//          rangeType(1:0..15): SEIndex,
//          origin(2:0..15): INTEGER,
//          range(3:0..15): CARDINAL],
//        long, real => [rangeType(1:0..15): SEIndex],
//        opaque => [
//          lengthKnown(0:8..15): BOOLEAN,
//          length(1:0..15): BitLength,
//          id(2:0..15): ISEIndex],
//        zone => [counted(0:8..8), mds(0:9..15): BOOLEAN],
//        any => [],
//        nil => [],
//        bits => [length(1:0..31): BitCount],   -- placed here to avoid
//        ENDCASE],         -- changing symbol version id's
//    ENDCASE];

struct SERecord : public MesaByteBuffer::HasRead, public HasToString {
    struct ID : public HasToString {
        struct TERMINAL : public HasToString  {
            std::string toString() const override {
                return "";
            }
        };
        struct SEQUENTIAL : public HasToString {
            std::string toString() const override {
                return "";
            }
        };
        struct LINKED : public MesaByteBuffer::HasRead, public HasToString {
            SEIndex link;

            MesaByteBuffer& read(MesaByteBuffer& bb) override {
                return bb.read(link);
            }
            std::string toString() const override {
                return link.toString();
            }
        };

        enum class Tag : uint16_t {
            ENUM_VALUE(Type, TERMINAL)
            ENUM_VALUE(Type, SEQUENTIAL)
            ENUM_VALUE(Type, LINKED)
        };
        static std::string toString(Tag value);

        bool     extended;
        bool     public_;
        CTXIndex idCtx;
        bool     immutable;
        bool     constant;
        SEIndex  idType;
        uint16_t idInfo;
        uint16_t idValue;
        HTIndex  hash;
        bool     linkSpace;
        Tag      tag;
        std::variant<TERMINAL, SEQUENTIAL, LINKED> variant;

        ID() : tag(Tag::TERMINAL), variant(TERMINAL{}) {}

        void read(uint16_t u0, MesaByteBuffer& bb);
        std::string toString() const override;

        DEFINE_VARIANT_METHOD(TERMINAL)
        DEFINE_VARIANT_METHOD(SEQUENTIAL)
        DEFINE_VARIANT_METHOD(LINKED)
    };
    struct CONS : public HasToString {
        struct MODE : public HasToString {
            std::string toString() const override {
                return "";
            }
        };
        struct BASIC : public HasToString {
            bool     ordered;
            uint16_t code;    // [0..16)
            uint16_t length;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct ENUMERATED : public HasToString {
            bool     ordered;
            bool     machineDep;
            bool     unpainted;
            bool     sparse;
            CTXIndex valueCtx;
            uint16_t nValues;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct RECORD : public HasToString {
            struct NOT_LINKED : public HasToString {
                std::string toString() const override {
                    return "";
                }
            };
            struct LINKED : public MesaByteBuffer::HasRead, public HasToString {
                SEIndex linkType;

                MesaByteBuffer& read(MesaByteBuffer& bb) override;
                std::string toString() const override;
            };

            enum class Tag : uint16_t {NOT_LINKED, LINKED};
            static std::string toString(Tag value);
            
            struct hints {
                bool comparable;
                bool assignable;
                bool unifield;
                bool variant;
                bool privateFields;
                bool refField;
                bool default_;
                bool voidable;
            };

            hints    hints;
            uint16_t length;
            bool     argument;
            bool     monitored;
            bool     machindDep;
            bool     painted;
            CTXIndex fieldCtx;
            Tag      tag;
            std::variant<NOT_LINKED, LINKED> variant;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;

            DEFINE_VARIANT_METHOD(NOT_LINKED)
            DEFINE_VARIANT_METHOD(LINKED)
        };
        struct REF : public HasToString {
            bool    counted;
            bool    ordered;
            bool    readOnly;
            bool    list;
            bool    var;
            bool    basing;
            SEIndex refType;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct ARRAY : public HasToString {
            bool    packed;
            SEIndex indexType;
            SEIndex componentType;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct ARRAYDESC : public HasToString {
            bool    var;
            bool    readOnly;
            SEIndex describedType;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct TRANSFER : public HasToString {
            bool         safe;
            TransferMode mode;
            SEIndex      typeIn;
            SEIndex      typeOut;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct DEFINITION : public HasToString {
            bool named;
            CTXIndex defCtx;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct UNION : public HasToString {
            uint16_t hints;
            bool     overlaid;
            bool     controlled;
            bool     machineDep;
            CTXIndex caseCtx;
            SEIndex  tagSei;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct SEQUENCE : public HasToString {
            bool    packed;
            bool    controlled;
            bool    machindDep;
            SEIndex tagSei;
            SEIndex componentType;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct RELATIVE : public HasToString {
            SEIndex baseType;
            SEIndex offsetType;
            SEIndex resultType;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct SUBRANGE : public HasToString {
            bool     filled;
            bool     empty;
            SEIndex  rangeType;
            int16_t  origin;
            uint16_t range;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct LONG : public HasToString {
            SEIndex rangeType;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct REAL : public HasToString {
            SEIndex rangeType;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct OPAQUE : public HasToString {
            bool     lengthKnown;
            uint16_t length;
            SEIndex  id;
            
            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct ZONE : public HasToString {
            bool counted;
            bool mds;

            void read(uint16_t u0, MesaByteBuffer& bb);
            std::string toString() const override;
        };
        struct ANY : public HasToString {
            std::string toString() const override {
                return "";
            }
        };
        struct NIL : public HasToString {
            std::string toString() const override {
                return "";
            }
        };
        struct BITS : public HasToString {
            u_int16_t length;

            void read(uint16_t u0, MesaByteBuffer& bb) {
                (void)u0;
                bb.read(length);
            }
            std::string toString() const override {
                return std_sprintf("%d", length);
            }
        };
        
        TypeClass tag;
        std::variant<
            MODE, BASIC, ENUMERATED, RECORD, REF,
            ARRAY, ARRAYDESC, TRANSFER, DEFINITION, UNION,
            SEQUENCE, RELATIVE, SUBRANGE, LONG, REAL,
            OPAQUE, ZONE, ANY, NIL, BITS
        > variant;

        void read(uint16_t u0, MesaByteBuffer& bb);
        std::string toString() const override;

        DEFINE_VARIANT_METHOD(MODE)
        DEFINE_VARIANT_METHOD(BASIC)
        DEFINE_VARIANT_METHOD(ENUMERATED)
        DEFINE_VARIANT_METHOD(RECORD)
        DEFINE_VARIANT_METHOD(REF)
        DEFINE_VARIANT_METHOD(ARRAY)
        DEFINE_VARIANT_METHOD(ARRAYDESC)
        DEFINE_VARIANT_METHOD(TRANSFER)
        DEFINE_VARIANT_METHOD(DEFINITION)
        DEFINE_VARIANT_METHOD(UNION)
        DEFINE_VARIANT_METHOD(SEQUENCE)
        DEFINE_VARIANT_METHOD(RELATIVE)
        DEFINE_VARIANT_METHOD(SUBRANGE)
        DEFINE_VARIANT_METHOD(LONG)
        DEFINE_VARIANT_METHOD(REAL)
        DEFINE_VARIANT_METHOD(OPAQUE)
        DEFINE_VARIANT_METHOD(ZONE)
        DEFINE_VARIANT_METHOD(ANY)
        DEFINE_VARIANT_METHOD(NIL)
        DEFINE_VARIANT_METHOD(BITS)
    };

    enum class Tag : uint16_t {
        ENUM_VALUE(Tag, ID)
        ENUM_VALUE(Tag, CONS)
    };
    static std::string toString(Tag value); // 01

    Tag                    tag;
    std::variant<ID, CONS> variant;

    SERecord() : tag(Tag::ID), variant(ID{}) {}

    MesaByteBuffer& read(MesaByteBuffer& bb) override;
    std::string toString() const override;

    DEFINE_VARIANT_METHOD(ID)
    DEFINE_VARIANT_METHOD(CONS)
};
