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
// ShowType.cpp
//

#include <cmath>
#include <functional>
#include <string>
#include <filesystem>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"

#include "../bcdFile/MDIndex.h"
#include "../bcdFile/MDRecord.h"
#include "../bcdFile/CTXRecord.h"
#include "../bcdFile/SERecord.h"

#include "SymbolOps.h"

#include "ShowType.h"

namespace ShowType {
//
static void NoSub() {}

template <class T>
static void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
    for (auto const& [key, value] : map) {
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
    }
}
Context::Context(const std::string& outDir, const std::string& bcdPath_) : bcdPath(bcdPath_), bcdFile(bcdPath) {
    auto bb = bcdFile.mesaByteBuffer();

    // set bcd
    bcd = BCD::getInstance(bb);

    // set symbol
    {
        const auto& symbolRange = bcd.symbolRange();
        if (symbolRange.size() != 1) ERROR();
        const auto& range = symbolRange.at(0);
        symbol = Symbol::getInstance(bb.range(range.offset, range.size));    
    }

    // for debug
    dumpTable("se", symbol.seTable);

    // set outPath
    {
        std::filesystem::path inputPath(bcdPath);
        std::filesystem::path outputPath = outDir;
        outputPath += inputPath.stem();
        outputPath += ".symbol";

        outPath = outputPath;    
        logger.info("outPath  %s", outPath);
    }

    // set module
    {
        const MDRecord& own = *symbol.mdTable[MDIndex::MD_OWN];
        const auto& filename = own.fileId.toValue();
        auto dotPos = filename.find(".");
        module = filename.substr(0, dotPos);
        logger.info("module  %s", module);    
    }

}


template<typename T>
bool isLast(T i, T end) {
    i++;
    return i == end;
}


void print(Context& context) {
    printHeader(context);
    printDirectory(context);
    printModule(context);
}

void printHeader(Context& context) {
    auto& bcd = context.bcd;
    auto& symbol = context.symbol;
    auto& out = context.out;

    // print header
    {
        const MDRecord& own = *symbol.mdTable[MDIndex::MD_OWN];
        out.println("----");
        out.println("--  File  %s  %s", bcd.sourceFile.value().version.toString(), own.fileId.toValue());
        out.println("----");
        out.println();
    }
}

void printDirectory(Context& context) {
    auto& symbol = context.symbol;
    auto& out = context.out;

    if (symbol.mdTable.size() == 1) return; // if only MD_OWN

	// Print Directory
    out.println("DIRECTORY");
    out.nest();

    auto begin = symbol.mdTable.cbegin();
    auto end = symbol.mdTable.cend();

    for(auto i = begin; i != end; i++) {
        auto& [index, value] = *i;
        if (index == MDIndex::MD_OWN) continue;
        auto& md = *value;
        out.println("%-30s %s%s", md.moduleId.toValue(), md.stamp.toString(), isLast(i, end) ? ";" : ",");
    }

    out.unnest();
    out.println();
}

void printModule(Context& context) {
    auto& symbol = context.symbol;
    auto& out = context.out;

    auto& own = *symbol.mdTable[MDIndex::MD_OWN];
    out.println("%s %s = BEGIN", own.stamp.toString(), own.moduleId.toValue());
    out.nest();

    for(auto sei = symbol.outerCtx.value().seList; !sei.isNull(); sei = SymbolOps::nextSe(symbol, sei)) {
        printSym(context, sei, ": ");
        out.println(";");
    }

    out.unnest();
    out.println("END.");
}


void printThis(Context& context, SEIndex sei) {
    putCurrentModuleDot(context);
    printSym(context, sei, ": ");
    context.out.println(";");
}

void putCurrentModuleDot(Context& context) {
    context.out.print("%s.", context.module);
}


static HTIndex htNull(HTIndex::HT_NULL, 0);
void printSei(Context& context, SEIndex sei) {
    printHti(context, sei.isNull() ? htNull : sei.value().toID().hash);
}
void printHti(Context& context, HTIndex hti) {
    context.out.print(hti.isNull() ? "(anonymous)" : hti.toValue());
}

void getBitSpec(Context& context, SEIndex isei, std::string& bitspec) {
    (void)context;
    const auto& id = isei.value().toID();
    BitAddress a = toBitAddr(id.idValue);
    uint16_t s = id.idInfo;
    bitspec.clear();
    bitspec += std_sprintf(" (%d", a.wd + 0);
    if (s) {
        bitspec += std_sprintf(":%d..%d", a.bd + 0, a.bd + s - 1);
    }
    bitspec += "): ";
}

void printSym(Context& context, SEIndex sei, const std::string& colongstring) {
    auto& out = context.out;
    auto& symbol = context.symbol;

    bool savePublic = context.defaultPublic;
    
    const auto& id = sei.value().toID();
    if (!id.hash.isNull()) {
        printSei(context, sei);
        out.print(colongstring);
    }
    if (id.public_ != context.defaultPublic) {
        context.defaultPublic = context.defaultPublic;
        out.print(context.defaultPublic ? "PUBLIC" : "PRIVATE");
    }
    if (id.idType.isType()) {
        auto typeSei = SymbolOps::toSei(symbol, id.idInfo);
        out.print("TYPE");
        if (typeSei.value().isCONS()) {
            auto& cons = typeSei.value().toCONS();
            if (cons.tag != TypeClass::OPAQUE) out.print(" = ");
        } else {
            out.print(" = ");
        }
        auto vf = printType(context, typeSei, {});
        printDefaultValue(context, sei, vf);
    } else {
        auto typeSei = id.idType;
        if (id.immutable && !id.constant) {
            auto xferMode = SymbolOps::xferMode(symbol, typeSei);
            if (xferMode == TransferMode::NONE || xferMode == TransferMode::PROCESS) out.print("READONLY ");
        }
        auto vf = printType(context, typeSei, NoSub);
        if (id.constant) {
            if (vf.isTRANSFER()) {
                auto transfer = vf.toTRANSFER();
                auto bti = SymbolOps::toBti(symbol, id.idInfo);
                out.print(" = ");
                if (!bti.isNull()) {
                    switch(transfer.mode) {
                    case TransferMode::SIGNAL:
                    case TransferMode::ERROR_:
                        out.print("CODE");
                        break;
                    default:
                        out.print("(procedure body)");
                        break;
                    }
                } else if (id.extended) out.print("(MACHINE CODE)");
                else printTypedVal(context, id.idValue, vf); // a hardcoded constant
            } else {
                out.print(" = ");
                if (id.extended) {
                    printTreeLink(context, SymbolOps::findExtension(symbol, sei).tree, vf, 0);
                } else {
                    auto underTypeSei = SymbolOps::underType(symbol, typeSei);
                    if (underTypeSei.value().toCONS().tag == TypeClass::SUBRANGE) {
                        const auto& subrange = underTypeSei.value().toCONS().toSUBRANGE();
                        printTypedVal(context, id.idValue + subrange.origin, vf);
                    }
                }
            }
        }
    }

    context.defaultPublic = savePublic;
}

ValFormat getValFormat(Context& context, SEIndex tsei) {
    auto& symbol = context.symbol;

    if (tsei.value().isID()) {
        return getValFormat(context, SymbolOps::underType(symbol, tsei));
    }
    if (tsei.value().isCONS()) {
        const auto& cons = tsei.value().toCONS();
        switch(cons.tag) {
        case TypeClass::BASIC:
        {
            switch(cons.toBASIC().code) {
            case Symbol::CODE_ANY:
                return ValFormat::getUNSIGNED();
            case Symbol::CODE_INT:
                return ValFormat::getSIGNED();
            case Symbol::CODE_CHAR:
                return ValFormat::getCHAR();
            default:
                ERROR()
            }
        }
        case TypeClass::ENUMERATED:
            return ValFormat::getENUM(tsei);
        case TypeClass::ARRAY:
            return ValFormat::getARRAY(cons.toARRAY().componentType);
        case TypeClass::TRANSFER:
            return ValFormat::getTRANSFER(cons.toTRANSFER().mode);
        case TypeClass::RELATIVE:
            return getValFormat(context, cons.toRELATIVE().offsetType);
        case TypeClass::SUBRANGE:
        {
            const auto& subrange = cons.toSUBRANGE();
            ValFormat vf = getValFormat(context, subrange.rangeType);
            if (vf.isSIGNED() && subrange.origin == 0) vf = ValFormat::getUNSIGNED();
            return vf;
        }
        case TypeClass::LONG:
            return getValFormat(context, cons.toLONG().rangeType);
        case TypeClass::REF:
            return ValFormat::getREF();
        default:
            return ValFormat::getOTHER();
        }
    }
    return ValFormat::getOTHER();
}

bool isVar(SEIndex tsei) {
    const auto& se = tsei.value();
    if (se.isCONS()) {
        const auto& cons = se.toCONS();
        if (cons.isREF()) return cons.toREF().var;
    }
    return false;
}

void putEnum(Context &context, uint16_t val, SEIndex esei) {
    const auto& symbol = context.symbol;
    auto& out = context.out;

    auto sei = SymbolOps::firstCtxSe(symbol, esei.value().toCONS().toENUMERATED().valueCtx);
    for(;;) {
        if (sei.isNull()) break;
        if (sei.value().toID().idValue == val) {
            printSei(context, sei);
            return;
        }
    }
    out.print("LOOPHOLE[%u]", val);
}

void putModeName(Context &context, TransferMode n) {
    static std::map<TransferMode, std::string> map = {
        {TransferMode::PROC,    "PROCEDURE"},
        {TransferMode::PORT,    "PORT"},
        {TransferMode::SIGNAL,  "SIGNAL"},
        {TransferMode::ERROR_,  "ERROR"},
        {TransferMode::PROCESS, "PROCESS"},
        {TransferMode::PROGRAM, "PROGRAM"},
    };
    context.out.print(map.at(n));
}

void outArgType(Context &context, SEIndex sei) {
    auto& out = context.out;
    if (sei.isNull()) out.print(": NIL");
    else {
        const auto& cons = sei.value().toCONS();
        if (cons.isRECORD()) printFieldCtx(context, cons.toRECORD().fieldCtx);
        if (cons.isANY()) out.print(" ANY ");
    }
}

void printFieldCtx(Context &context, CTXIndex ctx, bool md) {
    const auto& symbol = context.symbol;
    auto& out = context.out;

    auto isei = SymbolOps::firstCtxSe(symbol, ctx);
    std::string bitspec;
    if (!md) bitspec += ": ";
    if (!isei.isNull() && isei.value().toID().idCtx != ctx)
        isei = SymbolOps::nextSe(symbol, isei);
    out.print("[");
    bool first = true;
    for(; !isei.isNull(); isei = SymbolOps::nextSe(symbol, isei)) {
        if (first) first = false;
        else out.print(", ");

        if (md) getBitSpec(context, isei, bitspec);
        printSym(context, isei, bitspec);
        printDefaultValue(context, isei, getValFormat(context, isei.value().toID().idType));
    }
    out.print("]");
}

void printTypedVal(Context& context, uint16_t val, ValFormat vf) {
    context.out.print("<< %s >>", __FUNCTION__); // FIXME
    auto& out = context.out;

    bool loophole = false;
    switch (vf.tag) {
    case ValFormat::Tag::SIGNED:
        out.print("%d", val);
        break;
    case ValFormat::Tag::UNSIGNED:
        out.print("%u", val);
        break;
        case ValFormat::Tag::CHAR:
        out.print("%cC", val);
        break;
    case ValFormat::Tag::ENUM:
        putEnum(context, val, vf.toENUM().esei);
        break;
    case ValFormat::Tag::TRANSFER:
    case ValFormat::Tag::REF:
        if (val == 0) out.print("NIL");
        else loophole = true;
    default:
        loophole = true;
    }
    if (loophole) out.print("LOOPHOLE[%u]", val);
}

ValFormat printType(Context& context, SEIndex tsei, std::function<void()> dosub) {
    auto& out= context.out;
    auto& symbol = context.symbol;

    auto vf = getValFormat(context, tsei);

    switch(tsei.value().tag) {
    case SERecord::Tag::ID:
    {
        // print adjectives, if any
        for(;;) {
            auto tseiNext = SymbolOps::typeLink(symbol, tsei);
            if (tseiNext.isNull()) break;
            if (tsei.value().isID()) {
                printSei(context, tsei);
                out.print(" ");
            }
            tsei = tseiNext;
        }

        // print module qualification of last ID in chain
        const auto& id = tsei.value().toID();
        if (!id.idCtx.isStandardContext()) {
            switch(id.idCtx.value().tag) {
            case CTXRecord::Tag::INCLUDED:
                printHti(context, id.idCtx.value().toINCLUDED().module.value().moduleId);
                out.print(".");
                break;
            case CTXRecord::Tag::SIMPLE:
                putCurrentModuleDot(context);
                break;
            default:
                break;
            }
        }
        // finally print that last ID
        printSei(context, tsei);
        dosub();
    }
        break;
    case SERecord::Tag::CONS:
    {
        const auto& cons = tsei.value().toCONS();
        switch(cons.tag) {
            case TypeClass::ENUMERATED:
            ///////////////////////////////////////
            ///////////////////////////////////////
            ///////////////////////////////////////
                context.out.print("<< %s ENUMERATED >>", __FUNCTION__); // FIXME
                break;
            case TypeClass::RECORD:
            ///////////////////////////////////////
            ///////////////////////////////////////
            ///////////////////////////////////////
                context.out.print("<< %s RECORD >>", __FUNCTION__); // FIXME
                break;
            case TypeClass::REF:
            ///////////////////////////////////////
            ///////////////////////////////////////
            ///////////////////////////////////////
                context.out.print("<< %s REF >>", __FUNCTION__); // FIXME
                break;
            case TypeClass::ARRAY:
            {
                const auto& array = cons.toARRAY();
                if (array.packed) out.print("PACKED ");
                out.print("ARRAY ");
                printType(context, array.indexType, NoSub);
                out.print(" OF ");
                printType(context, array.componentType, NoSub);
            }
                break;
            case TypeClass::ARRAYDESC:
            {
                const auto& arraydesc = cons.toARRAYDESC();
                out.print("DESCRIPTOR FOR ");
                if (arraydesc.readOnly) out.print("READONLY ");
                printType(context, arraydesc.describedType, NoSub);
            }
                break;
            case TypeClass::TRANSFER:
            {
                const auto& transfer = cons.toTRANSFER();
                putModeName(context, transfer.mode);
                if (!transfer.typeIn.isNull()) {
                    out.print(" ");
                    outArgType(context, transfer.typeIn);
                }
                if (!transfer.typeOut.isNull()) {
                    out.print(" RETURNS ");
                    outArgType(context, transfer.typeOut);
                }
            }
                break;
            case TypeClass::UNION:
            ///////////////////////////////////////
            ///////////////////////////////////////
            ///////////////////////////////////////
                context.out.print("<< %s UNION >>", __FUNCTION__); // FIXME
                break;
            case TypeClass::RELATIVE:
            {
                const auto& relative = cons.toRELATIVE();
                if (!relative.baseType.isNull()) printType(context, relative.baseType, NoSub);
                out.print(" RELATIVE ");
                printType(context, relative.offsetType, dosub);
            }
                break;
            case TypeClass::SEQUENCE:
            {
                const auto& sequnce = cons.toSEQUENCE();
                if (sequnce.packed) out.print("PACKED ");
                out.print("SEQUENCE ");
                if (!sequnce.controlled) out.print("COMPUTED ");
                else {
                    printSei(context, sequnce.tagSei);
                    if (sequnce.machindDep || context.showBits) {
                        std::string bitspec;
                        getBitSpec(context, sequnce.tagSei, bitspec);
                        out.print(bitspec);
                    } else out.print(": ");
                }
                SEIndex tagType = sequnce.tagSei.value().toID().idType;
                
                if (sequnce.tagSei.value().toID().public_ != context.defaultPublic) {
                    out.print(context.defaultPublic ? "PRIVATE " : "PUBLID ");
                }
                switch(tagType.value().tag) {
                    case SERecord::Tag::ID:
                        printType(context, tagType, NoSub);
                        break;
                    case SERecord::Tag::CONS:
                        out.print("*");
                        break;
                    default:
                        ERROR();
                }
                printType(context, tagType, NoSub);
                out.print(" OF ");
                printType(context, sequnce.componentType, NoSub);
            }
                break;
            case TypeClass::SUBRANGE:
            {
                const auto& subtrange = cons.toSUBRANGE();
                int16_t org = subtrange.origin;
                uint16_t size = subtrange.range;

                auto doit = [&]{
                    ValFormat vfSub;
                    if (vf.isENUM()) vfSub = vf;
                    else if (org < 0) vfSub = ValFormat::getSIGNED();
                    else vfSub = ValFormat::getUNSIGNED();
                    out.print("[");
                    printTypedVal(context, org, vfSub);
                    out.print("..");
                    if (subtrange.empty) {
                        printTypedVal(context, org, vfSub);
                        out.print(")");
                    } else {
                        printTypedVal(context, org + size, vfSub);
                        out.print("]");
                    }
                };
                printType(context, subtrange.rangeType, doit);
            }
                break;
            case TypeClass::ZONE:
            {
                const auto& zone = cons.toZONE();
                if (zone.counted) out.print("ZONE");
                else if (zone.mds) out.print("MDSZone");
                else out.print("UNCOUNTED ZONE");
            }
                break;
            case TypeClass::OPAQUE:
                {
                    const auto& opaque = cons.toOPAQUE();
                    if (opaque.lengthKnown) {
                        out.print("[%u]", opaque.length / Environment::bitsPerWord);
                    }
                }
                break;
            case TypeClass::LONG:
            {
                auto rangeType = cons.toLONG().rangeType;
                if (!ShowType::isVar(rangeType)) out.print("LONG ");
                printType(context, rangeType, NoSub);
            }
                break;
            case TypeClass::REAL:
                out.print("REAL");
                break;
            default:
                ERROR()
                break;
        }
    }
        break;
    default:
        ERROR()
    }
    
    return vf;
}

void printDefaultValue(Context& context, SEIndex sei, ValFormat vf) {
    (void)sei, (void)vf;
    context.out.print("<< %s >>", __FUNCTION__); // FIXME
}

void printTreeLink(Context& context, TreeLink tree, ValFormat vf, int recur, bool sonOfDot) {
    (void)tree, (void)vf, (void)recur, (void)sonOfDot;
    context.out.print("<< %s >>", __FUNCTION__); // FIXME
}

}

