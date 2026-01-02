/*******************************************************************************
 * Copyright (c) 2026, Yasuhiro Hasegawa
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

#include "../util/StringPrinter.h"

#include "BCDFile.h"
#include "BCD.h"
#include "Symbol.h"

#include "Type.h"
#include "SEIndex.h"
#include <functional>
#include <variant>

namespace print_symbol {

template<typename T>
bool isLast(T i, T end) {
    i++;
    return i == end;
}

struct Context {
    std::string   bcdPath;
    BCDFile       bcdFile;
    BCD           bcd;
    Symbol        symbol;

    std::string   outPath;
    StringPrinter out;

    Context(const std::string& outDir, const std::string& bcdPath);
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
    enum class Tag {SIGNED, UNSIGNED, CHAR, ENUM, ARRAY, TRANSFER, REF, OTHER};

    struct SIGNED {};
    struct UNSIGNED {};
    struct CHAR {};
    struct ENUM {
        SEIndex esei;
        ENUM(SEIndex esei_) : esei(esei_) {}
    };
    struct ARRAY {
        SEIndex componentType;
        ARRAY(SEIndex componentType_) : componentType(componentType_) {}
    };
    struct TRANSFER {
        TransferMode mode;
        TRANSFER(TransferMode mode_) : mode(mode_) {}
    };
    struct REF {};
    struct OTHER {};
    using VariantType = std::variant<SIGNED, UNSIGNED, CHAR, ENUM, ARRAY, TRANSFER, REF, OTHER>;

    Tag tag;
    VariantType variant;

    ValFormat() : tag(Tag::OTHER), variant(OTHER{}) {}
    ValFormat(Tag tag_, VariantType variant_) : tag(tag_), variant(variant_) {}

    static ValFormat getUNSIGNED() {
        ValFormat ret(Tag::UNSIGNED, UNSIGNED{});
        return ret;
    }
    static ValFormat getSIGNED() {
        ValFormat ret(Tag::SIGNED, SIGNED{});
        return ret;
    }
    static ValFormat getCHAR() {
        ValFormat ret(Tag::CHAR, CHAR{});
        return ret;
    }
    static ValFormat getENUM(SEIndex esei) {
        ValFormat ret(Tag::ENUM, ENUM(esei));
        return ret;
    }
    static ValFormat getARRAY(SEIndex componentType) {
        ValFormat ret(Tag::ARRAY, ARRAY(componentType));
        return ret;
    }
    static ValFormat getTRANSFER(TransferMode mode) {
        ValFormat ret(Tag::ARRAY, TRANSFER(mode));
        return ret;
    }
    static ValFormat getREF() {
        ValFormat ret(Tag::REF, REF{});
        return ret;
    }
    static ValFormat getOTHER() {
        ValFormat ret(Tag::OTHER, OTHER{});
        return ret;
    }


    const ENUM& toENUM () {
        return std::get<ENUM>(variant);
    }
    const ARRAY& toARRAY () {
        return std::get<ARRAY>(variant);
    }
    const TRANSFER& toTRANSFER () {
        return std::get<TRANSFER>(variant);
    }
};


void printHeader(Context& context);
void printDirectory(Context& context);
void printModule(Context& context);

void printSymbol(Context& context, SEIndex sei, const std::string& colonString);

inline void print(Context& context) {
    printHeader(context);
    printDirectory(context);
    printModule(context);
}

ValFormat getValFormat(Context& context, SEIndex tsei);
ValFormat printType(Context& context, SEIndex sei, std::function<void()> dosub);
void printDefaultValue(Context& context, SEIndex sei, ValFormat vf);

}
