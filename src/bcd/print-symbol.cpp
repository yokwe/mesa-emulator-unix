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
// print-symbol.cpp
//

#include <iterator>
#include <string>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/StringPrinter.h"

#include "BCDFile.h"
#include "BCD.h"
#include "Symbols.h"
#include "SymbolsIndex.h"

#include "MDRecord.h"
#include "HTRecord.h"
#include "CTXRecord.h"
#include "SERecord.h"
#include "BTRecord.h"
#include "EXTRecord.h"

#include "print-symbol.h"

namespace PrintSymbol {

void print(const std::string& path) {
    Context context;
    
    BCDFile bcdFile(path);
    ByteBuffer bb = bcdFile.getByteBuffer();

    context.bcd = BCD::getInstance(bb);
    if (!context.bcd.hasSymbol()) ERROR()
    context.symbols = Symbols::getInstance(bb, context.bcd.getSymbolOffset());
    context.symbols.dumpTable();

    print(context);
}
void print(Context& context) {
    auto& bcd = context.bcd;
    auto& symbols = context.symbols;
    auto& out = context.out;

    // print header
    {
        const MDRecord& own = *symbols.mdTable[MDIndex::MD_OWN];
        out.println("----");
        out.println("--  File  %s  %s", bcd.sourceFile.value().version.toString(), own.fileId.toValue());
        out.println("----");
        out.println();
    }

    // print directory
    {
        int mdSize = symbols.mdTable.size();
        if (1 < mdSize) {
            out.println("DIRECTORY");
            out.nest();
            for(auto i = symbols.mdTable.cbegin(); i != symbols.mdTable.cend(); i++) {
                const auto key = i->first;
                const auto& value = *i->second;
                if (key == MDIndex::MD_OWN) continue;
                out.println("%s %s%s",
                    value.stamp.toString(),
                    value.moduleId.toValue(),
                    std::next(i) != symbols.mdTable.cend() ? "," : ";"
                );
            }
            out.unnest();
        }
    }

    // print module
    {
        const MDRecord& own = *symbols.mdTable[MDIndex::MD_OWN];
        out.println("%s %s = BEGIN",
            own.stamp.toString(),
            own.moduleId.toValue());
    }

    {
        out.nest();
        const CTXRecord& outerCtx = symbols.outerCtx.value();
        out.println("// ctx  %s - %s", outerCtx.toString(), toString(outerCtx.level));
        out.println("// seList  %s  %s", outerCtx.seList.toString(), outerCtx.seList.value().toString());

        for(SEIndex sei = outerCtx.seList; !sei.isNull(); sei = symbols.nextSei(sei)) {
//            logger.info("sei  %s  %s", sei.toString(), sei.value().toString());
            printSym(context, sei, ": ");
            out.println(";");
        }
        out.unnest();
        out.println("END.");
    }
}

void printSym(Context& context, const SEIndex sei, std::string colonString) {
    auto& bcd = context.bcd;
    auto& symbols = context.symbols;
    auto& out = context.out;
    (void)bcd;

    out.print("*printSym*  ");

    using ID = SERecord::ID;
//    const ID id = std::get<ID>(sei.value().body);
    const ID id = sei.value().toID();
    if (!id.hash.isNull()) out.print("%s%s", id.hash.toValue(), colonString);
    if (!id.public_) out.print("PRIVATE ");

    if (id.idType.isType()) {
        const SEIndex typeSei = symbols.toSEIndex(id.idInfo);
		out.print("TYPE");

        if (typeSei.value().tag == SERecord::Tag::CONS) {
            if (typeSei.value().toCONS().tag != TypeClass::OPAQUE) out.print(" = ");
        } else {
            out.print(" = ");
        }

		ValFormat vf = printType(context, typeSei, {});
		printDefaultValue(context, sei, vf);
    } else {
        const SEIndex typeSei = id.idType;
		if (id.immutable && (!id.constant)) {
            switch(symbols.xferMode(typeSei)) {
            case TransferMode::NONE:
            case TransferMode::PROCESS:
                out.print("READONLY ");
                break;
            default:
                break;
            }
		}
        ValFormat vf = printType(context, typeSei, {});
        if (id.constant) {
            out.print(" = ");
            if (vf.tag == ValFormat::Tag::TRANSFER) {
                BTIndex bti = symbols.toBTIndex(id.idInfo);
                if (bti.hasValue()) {
                    auto mode = vf.toTRANSFER().mode;
                    if (mode == TransferMode::SIGNAL || mode == TransferMode::ERROR_) {
                        out.print("CODE");
                    } else if (bti.value().toCALLABLE().inline_) {
                        out.print("INLINE");
                    } else {
                        out.print("BODY");
                    }
                } else if (sei.value().toID().extended) {
                    out.print("OPCODE");
                } else {
                    printTypedVal(context, sei.value().toID().idValue, vf);
                }
            } else {
                if (sei.value().toID().extended) {
                    // FIXME failed at inside toEXTIndex()
                    const EXTRecord& ext = symbols.toEXTIndex(sei).value();
                    printTreeLink(context, ext.tree, vf, 0, false);
                    out.print("*extended*");
                } else {
                    const auto underTypeSei = symbols.underType(typeSei);
                    uint16_t index = sei.value().toID().idValue;
                    if (underTypeSei.value().toCONS().tag == TypeClass::SUBRANGE) {
                        index += underTypeSei.value().toCONS().toSUBRANGE().origin;
                    }
                    printTypedVal(context, index, vf);
                }
            }
        }
    }
}

ValFormat printType(Context& context, const SEIndex tsei, std::function<void()> dosub) {
    (void)context; (void)tsei; (void)dosub;
    context.out.print("*%s*", __FUNCTION__);

    // FIXME

    ValFormat valFormat;

    return valFormat;
}

void printDefaultValue(Context& context, const SEIndex sei, ValFormat valFormat) {
    (void)context; (void)sei, (void)valFormat;
    // FIXME
    context.out.print("*%s*", __FUNCTION__);
}

void printTypedVal(Context& context, uint16_t index, ValFormat vf) {
    (void)context; (void)index; (void)vf;
    // FIXEME
    context.out.print("*%s*", __FUNCTION__);
}

void printTreeLink(Context& context, const TreeLink tree, ValFormat vf, int recur, bool sonOfDot) {
    (void)context; (void)tree; (void)vf; (void)recur; (void)sonOfDot;
    context.out.print("*%s*", __FUNCTION__);
}
}