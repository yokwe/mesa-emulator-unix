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
// BodyRecord.h
//

#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "../util/Util.h"

#include "../util/ByteBuffer.h"

#include "Type.h"
#include "BTIndex.h"
#include "TreeIndex.h"
#include "SEIndex.h"
#include "CTXIndex.h"

// NOTICE
//   size of BodyLink is 15 bits
//BodyLink: TYPE = RECORD [which(0:0..0): {sibling(0), parent(1)}, index(0:1..14): BTIndex];
struct BodyLink : public ByteBuffer::HasRead, public HasToString {
    enum class Which {
        ENUM_VALUE(Which, SIBLING)
        ENUM_VALUE(Which, PARENT)
    };
    static std::string toString(Which);

    Which   which;
    BTIndex index;

    BodyLink() : which(Which::SIBLING) {}

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};


//BodyInfo: TYPE = RECORD [
//  SELECT mark(0:0..0): * FROM
//    Internal => [
//      bodyTree(0:1..15): Base RELATIVE POINTER [0..Limit),
//        --Tree.Index--
//      thread(1:0..15): Base RELATIVE POINTER [0..Limit),
//        --Tree.Index / LitDefs.STIndex--
//      frameSize(2:0..15): [0..PrincOps.MaxFrameSize]],
//    External => [
//      bytes(0:1..15): [0..LAST[CARDINAL]/2],
//      startIndex(1:0..15), indexLength(2:0..15): CARDINAL]
//    ENDCASE];
struct BodyInfo : public ByteBuffer::HasRead, public HasToString {
    enum class Tag {
        ENUM_VALUE(Tag, INTERNAL)
        ENUM_VALUE(Tag, EXTERNAL)
    };
    static std::string toString(Tag);

    struct INTERNAL : public HasToString {
        TreeIndex bodyTree;
        TreeIndex thread;
        uint16_t  frameSize;

        void read(uint16_t u0, ByteBuffer& bb);
        std::string toString() const override;
    };
    struct EXTERNAL : public HasToString {
        uint16_t bytes;
        uint16_t startIndex;
        uint16_t indexLength;

        void read(uint16_t u0, ByteBuffer& bb);
        std::string toString() const override;
    };

    Tag tag;
    std::variant<INTERNAL, EXTERNAL> variant;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};


//CatchIndex: TYPE = NATURAL;
//BodyRecord: TYPE = RECORD [
//  link(0:0..15): BodyLink,
//  firstSon(1:0..15): BTIndex,
//  type(2:0..15): RecordSEIndex,
//  localCtx(3:0..10): CTXIndex,
//  level(3:11..15): ContextLevel,
//  sourceIndex(4:0..15): CARDINAL,
//  info(5:0..47): BodyInfo,
//  extension(8:0..79): SELECT kind: * FROM
//    Callable => [
//      inline(8:1..1): BOOLEAN,
//      id(8:2..15): ISEIndex,
//      ioType(9:0..13): CSEIndex,
//      monitored(9:14..14), noXfers(9:15..15), resident(10:0..0): BOOLEAN,
//      entry(10:1..1), internal(10:2..2): BOOLEAN,
//      entryIndex(10:3..10): [0..256),
//      hints(10:11..15): RECORD [safe(0:0..0), argUpdated(0:1..1), nameSafe(0:2..2), needsFixup(0:3..3): BOOLEAN],
//      closure(11:0..31): SELECT nesting(11:0..1): * FROM
//        Outer => [],
//        Inner => [frameOffset(11:2..15): [0..PrincOps.MaxFrameSize)],
//        Catch => [index(12:0..15): CatchIndex]
//        ENDCASE],
//    Other => [relOffset(8:1..15): [0..LAST[CARDINAL]/2]]
//    ENDCASE];

struct BodyRecord : public ByteBuffer::HasRead, public HasToString {
    enum class Tag {
        ENUM_VALUE(Tag, CALLABLE)
        ENUM_VALUE(Tag, OTHER)
    };
    static std::string toString(Tag);

    struct CALLABLE : public HasToString {
        enum class Tag : uint16_t {
            ENUM_VALUE(Tag, OUTER)
            ENUM_VALUE(Tag, INNER)
            ENUM_VALUE(Tag, CATCH)
        };
        static std::string toString(Tag);

        struct OUTER : public HasToString {
            void read(uint16_t u11, ByteBuffer& bb);
            std::string toString() const override {
                return "";
            }
        };
        struct INNER : public HasToString {
            uint16_t frameOffset;

            void read(uint16_t u11, ByteBuffer& bb);
            std::string toString() const override {
                return std_sprintf("%d", frameOffset);
            }
        };
        struct CATCH : public HasToString {
            uint16_t index;

            void read(uint16_t u11, ByteBuffer& bb);
            std::string toString() const override {
                return std_sprintf("%d", index);
            }
        };

        bool     inline_;
        SEIndex  id;
        SEIndex  ioType;
        bool     monitored;
        bool     noXfers;
        bool     resident;
        bool     entry;
        bool     internal;
        uint16_t entryIndex;
        uint16_t hints;
        Tag      tag;
        std::variant<OUTER, INNER, CATCH> variant;

        void read(uint16_t u8, ByteBuffer& bb);
        std::string toString() const override;

        DEFINE_VARIANT_METHOD(OUTER)
        DEFINE_VARIANT_METHOD(INNER)
        DEFINE_VARIANT_METHOD(CATCH)
    
    };
    struct OTHER : public HasToString {
        uint16_t relOffset;

        void read(uint16_t u8, ByteBuffer& bb);
        std::string toString() const override;
    };

    BodyLink     link;
    BTIndex      firstSon;
    SEIndex      type;
    CTXIndex     localCtx;
    ContextLevel level;
    uint16_t     sourceIndex;
    BodyInfo     info;
    Tag          tag;
    std::variant<CALLABLE, OTHER> variant;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;

    DEFINE_VARIANT_METHOD(CALLABLE)
    DEFINE_VARIANT_METHOD(OTHER)
};