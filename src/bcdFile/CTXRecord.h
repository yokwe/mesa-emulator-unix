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
// CTXRecord.h
//

#pragma once

#include <cstdint>
#include <string>

#include "../util/Util.h"

#include "MesaByteBuffer.h"

#include "Type.h"
#include "MDIndex.h"
#include "CTXIndex.h"
#include "SEIndex.h"


// CTXRecord: TYPE = RECORD [
//   mark(0:0..0), varUpdated(0:1..1): BOOLEAN,
//   seList(0:2..15): ISEIndex,
//   level(1:0..2): ContextLevel,
//   extension(1:3..47): SELECT ctxType(1:3..4): * FROM
//     simple => [ctxNew(1:5..15): CTXIndex], -- for DeSoto
//     included => [
//       chain(1:5..15): IncludedCTXIndex,
//       copied(2:0..1): Closure � none,
//       module(2:2..15): MDIndex,
//       map(3:0..10): CTXIndex,
//       closed(3:11..11), complete(3:12..12), restricted(3:13..13): BOOLEAN,
//       reset(3:14..15): BOOLEAN],
//     imported => [includeLink(1:5..15): IncludedCTXIndex],
//     nil => []
//     ENDCASE];
struct CTXRecord : public MesaByteBuffer::HasRead, public HasToString {
    //Closure: TYPE = {none, unit, rc, full};  -- completeness of copied contexts
    enum class Closure : uint16_t {
        ENUM_VALUE(Closure, NONE)
        ENUM_VALUE(Closure, UNIT)
        ENUM_VALUE(Closure, RC)
        ENUM_VALUE(Closure, FULL)
    };
    static std::string toString(Closure value);

    struct SIMPLE : public HasToString {
        CTXIndex ctxNew;

        SIMPLE(uint16_t u1);
        std::string toString() const override;
    };
    struct INCLUDED : public HasToString {
        CTXIndex chain;
        Closure  copied;
        MDIndex  module;
        CTXIndex map;
        bool     closed, complete, restricted;
        bool     reset;
        
        INCLUDED(uint16_t u1, MesaByteBuffer& bb);
        std::string toString() const override;
    };
    struct IMPORTED : public HasToString {
        CTXIndex includeIndex;

        IMPORTED(uint16_t u1);
        std::string toString() const override ;
    };
    struct NIL : public HasToString {
        std::string toString() const override {
            return "{}";
        }
    };

    enum class Tag {
        ENUM_VALUE(Type, SIMPLE)
        ENUM_VALUE(Type, INCLUDED)
        ENUM_VALUE(Type, IMPORTED)
        ENUM_VALUE(Type, NIL)
    };
    static std::string toString(Tag value);

    bool         mark;
    bool         varUpdated;
    SEIndex      seList;
    ContextLevel level;
    Tag          tag;
    std::variant<SIMPLE, INCLUDED, IMPORTED, NIL> variant;

    CTXRecord() : mark(false), varUpdated(false), seList(), level(ContextLevel::LZ), tag(Tag::NIL), variant(NIL{}) {}

    MesaByteBuffer& read(MesaByteBuffer& bb) override;
    std::string toString() const override;

    const SIMPLE&   toSIMPLE() {
        return std::get<CTXRecord::SIMPLE>(variant);
    }
    const INCLUDED& toINCLUDED() {
        return std::get<CTXRecord::INCLUDED>(variant);
    }
    const IMPORTED& toIMPORTED() {
        return std::get<CTXRecord::IMPORTED>(variant);
    }
    const NIL&      toNIL() {
        return std::get<CTXRecord::NIL>(variant);
    }
};
