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
// PrintSymbol.h
//

#pragma once

#include <string>
#include <variant>

#include "../util/Util.h"
#include "../util/StringPrinter.h"

#include "BCD.h"
#include "Symbols.h"
#include "Tree.h"


namespace PrintSymbol {

struct Context {
    BCD           bcd;
    Symbols       symbols;
    StringPrinter out;

    SEIndex findSEIndex(uint16_t index);
};

//EnumeratedSEIndex: TYPE = Table.Base RELATIVE POINTER [0..Table.Limit)
//  TO enumerated cons Symbols.SERecord;
//
//ValFormat: TYPE = RECORD [
//  SELECT tag: * FROM
//    signed => [], --an INTEGER or subrange with base < 0
//    unsigned => [], -- a CARDINAL, WORD, UNSPECIFIED, or subrange w/ base >= 0
//    char => [], --a character
//    enum => [esei: EnumeratedSEIndex], --an enumerated type
//    array => [componentType: Symbols.SEIndex],
//    transfer => [mode: Symbols.TransferMode], --a PROCEDURE, SIGNAL, ERROR, PROGRAM, or PORT
//    ref => [], --a pointer
//    other => [], --anything else (whether single word or multi-word)
//    ENDCASE];
struct ValFormat {
    enum class Tag {
        ENUM_VALUE(Type, SIGNED)
        ENUM_VALUE(Type, UNSIGNED)
        ENUM_VALUE(Type, CHAR)
        ENUM_VALUE(Type, ENUM)
        ENUM_VALUE(Type, ARRAY)
        ENUM_VALUE(Type, TRANSFER)
        ENUM_VALUE(Type, REF)
        ENUM_VALUE(Type, OTHER)
    };
    struct SIGNED {};
    struct UNSIGNED {};
    struct CHAR {};
    struct ENUM {
        SEIndex esei;
    };
    struct ARRAY {
        SEIndex componentType;
    };
    struct TRANSFER {
        TransferMode mode;
    };
    struct REF {};
    struct OTHER {};

    Tag tag;
    std::variant<SIGNED, UNSIGNED, CHAR, ENUM, ARRAY, TRANSFER, REF, OTHER> variant;

    ValFormat(SIGNED value_) : tag(Tag::SIGNED), variant(value_) {}
    ValFormat(UNSIGNED value_) : tag(Tag::UNSIGNED), variant(value_) {}
    ValFormat(CHAR value_) : tag(Tag::CHAR), variant(value_) {}
    ValFormat(ENUM value_) : tag(Tag::ENUM), variant(value_) {}
    ValFormat(ARRAY value_) : tag(Tag::ARRAY), variant(value_) {}
    ValFormat(TRANSFER value_) : tag(Tag::TRANSFER), variant(value_) {}
    ValFormat(REF value_) : tag(Tag::REF), variant(value_) {}
    ValFormat(OTHER value_) : tag(Tag::OTHER), variant(value_) {}

    ValFormat() : ValFormat(OTHER{}) {}

    TRANSFER toTRANSFER() const {
        return std::get<TRANSFER>(variant);
    }

};


void print(const std::string& path);

void print(Context& context);

void printSym(Context& cntext, const SEIndex sei, std::string colonString);

ValFormat printType(Context& cotnext, const SEIndex sei, std::function<void()> dosub);

void printDefaultValue(Context& context, const SEIndex sei, ValFormat valFormat);

void printTypedVal(Context& context, uint16_t index, ValFormat vf);

//PrintTreeLink: PROCEDURE [tree: Tree.Link, vf: ValFormat, recur: CARDINAL, sonOfDot: BOOLEAN � FALSE] =
void printTreeLink(Context& context, const TreeLink tree, ValFormat vf, int recur, bool sonOfDot = false);

}