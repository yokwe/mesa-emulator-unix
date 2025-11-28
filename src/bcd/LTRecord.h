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
// LTRecord.h
//

#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "../util/Util.h"

#include "Symbols.h"
#include "SymbolsIndex.h"

//WordSequence: TYPE = ARRAY [0..0) OF WORD;
//
//LTRecord: TYPE = RECORD [
//  link(0:0..12): LTIndex,
//  datum(0:13..47): SELECT kind(0:13..15): * FROM
//    short => [value(1:0..15): WORD],
//    long => [
//      codeIndex(1:0..15): [0..LAST[CARDINAL]/2],
//      length(2:0..15): CARDINAL,
//      value(3): WordSequence]
//    ENDCASE];
struct LTRecord : public ByteBuffer::Readable, public HasToString {
    enum class Kind : uint16_t {
        ENUM_VALUE(Kind, SHORT)
        ENUM_VALUE(Kind, LONG)
    };
    static std::string toString(Kind);

    struct SHORT : public ByteBuffer::Readable, public HasToString {
        uint16_t value;

        ByteBuffer& read(ByteBuffer& bb) override {
            return bb.read(value);
        }
        std::string toString() const override {
            return std_sprintf("[%d]", value);
        }
    };
    struct LONG : public ByteBuffer::Readable, public HasToString {
        uint16_t             codeIndex;
        uint16_t             length;
        std::vector<uint8_t> value;

        ByteBuffer& read(ByteBuffer& bb) override;
        std::string toString() const override;
    };

	// const LTIndex* link
	//   NOTE
	//   According to "Mesa Compiler Internal Documentation", Below is excerpt from Chap. B.9.3. Literal Table
    //   ----
    //     The link field points to the next LTRecord in the list of literals that map to the same index
    //     in the vector of LTindexes. Since literals are only accessed from this vector during
    //     compilation, the link field is unnecessary after compilation.
    //   ----
	//   Remove declaration from LTRecord
    // LTIndex link;
    Kind    kind;
    std::variant<SHORT, LONG> datum;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};

// NORICE
//   size of LitRecod is 14 bits
//   needs to shift 2 bit for 16 bit aligned data
//LitRecord: TYPE = RECORD [
//  SELECT litTag(0:0..0): * FROM
//    word => [index(0:1..13): LTIndex],
//    string => [index(0:1..13): STIndex]
//    ENDCASE];
struct LitRecord : public HasToString {
    enum class Tag : uint16_t {
        ENUM_VALUE(Tag, WORD)
        ENUM_VALUE(Tag, STRING)
    };
    static std::string toString(Tag);

    struct WORD : public HasToString {
        LTIndex index;

        void read(uint16_t u0) {
            index.index(bitField(u0, 1, 13));
        }
        std::string toString() const override {
            return std_sprintf("[%s]", index.Index::toString());
        }
    };
    struct STRING : public HasToString {
        uint16_t index; // STIndex

        void read(uint16_t u0) {
            index = bitField(u0, 1, 13);
        }
        std::string toString() const override {
            return std_sprintf("[%d]", index);
        }
    };
    Tag litTag;
    std::variant<WORD, STRING> value;

    void read(uint16_t u0);
    std::string toString() const override;
};
