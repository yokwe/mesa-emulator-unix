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
// ShowType.h
//

#pragma once

#include <functional>
#include <variant>

#include "../util/StringPrinter.h"

#include "../bcdFile/BCDFile.h"
#include "../bcdFile/BCD.h"
#include "../bcdFile/Symbol.h"

#include "../bcdFile/Type.h"
#include "../bcdFile/SEIndex.h"
#include "../bcdFile/Tree.h"

namespace ShowType {

struct Context {
    std::string   bcdPath;
    BCDFile       bcdFile;
    BCD           bcd;
    Symbol        symbol;

    std::string   outPath;
    StringPrinter out;

    // from ITShowType
    std::string   module;

    bool          defaultPublic = true; // outer RECORD is public or private?
    bool          showBits      = true; // if TRUE, show bit positions even if not MACHINE DEPENDENT

    Context(const std::string& outDir, const std::string& bcdPath);
};

// This is top level function
void print(Context& context);

void printHeader(Context& context);
void printDirectory(Context& context);
void printModule(Context& context);


//
// AMesa/14.0/Sword/Private/ITShowType.mesa
//

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

    using Variant = std::variant<std::monostate, ENUM, ARRAY, TRANSFER>;

    Tag     tag;
    Variant variant;

    ValFormat(Tag tag_, Variant variant_) : tag(tag_), variant(variant_) {}
    ValFormat(Tag tag_) : tag(tag_) {}


    static ValFormat getSIGNED() {
        return ValFormat(Tag::SIGNED);
    }
    static ValFormat getUNSIGNED() {
        return ValFormat(Tag::UNSIGNED);
    }
    static ValFormat getCHAR() {
        return ValFormat(Tag::CHAR);
    }
    static ValFormat getENUM(SEIndex esei) {
        return ValFormat(Tag::ENUM, ENUM{esei});
    }
    static ValFormat getARRAY(SEIndex componentType) {
        return ValFormat(Tag::ARRAY, ARRAY{componentType});
    }
    static ValFormat getTRANSFER(TransferMode mode) {
        return ValFormat(Tag::ARRAY, TRANSFER{mode});
    }
    static ValFormat getREF() {
        return ValFormat(Tag::REF);
    }
    static ValFormat getOTHER() {
        return ValFormat(Tag::OTHER);
    }

    const ENUM& toENUM() {
        return std::get<ENUM>(variant);
    }
    const ARRAY& toARRAY() {
        return std::get<ARRAY>(variant);
    }
    const TRANSFER& toTRANSFER() {
        return std::get<TRANSFER>(variant);
    }
};

void printThis(Context& context, SEIndex sei);

void printSym(Context& context, SEIndex sei, const std::string& colongstring);
void printTyedVal(Context& context, uint16_t val, ValFormat vf);
void getBitSpec(Context& context, SEIndex isei, std::string& bitspec);
void outArgType(Context& context, SEIndex sei);
void printFieldCtx(Context& context, CTXIndex ctx, bool md = false);
void printHti(Context& context, HTIndex hti);
void printSei(Context& context, SEIndex sei);
void putEnum(Context& context, uint16_t val, SEIndex esei);
ValFormat getValFormat(SEIndex tsei);
ValFormat printType(Context& context, SEIndex tsei, std::function<void()> dosub);
bool isVar(SEIndex tsei);
void putModeName(Context& context, TransferMode n);
void printDefaultValue(Context& context, SEIndex sei, ValFormat vf);
void printTreeLink(Context& context, TreeLink tree, ValFormat vf, int recur, bool sonOfDot = false);
void putCurrentModuleDot(Context& context);


}