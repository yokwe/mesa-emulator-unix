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
#include "../bcdFile/BodyRecord.h"
#include "../bcdFile/LTIndex.h"
#include "../bcdFile/LTRecord.h"

#include "SymbolOps.h"

#include "ShowType.h"

namespace ShowType {
//
template <class T>
static void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
    for (auto const& [key, value] : map) {
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
    }
}
Context::Context(const std::string& outDir, const std::string& bcdPath_) : bcdPath(bcdPath_), bcdFile(bcdPath) {
    auto bb = bcdFile.byteBuffer();

    // set bcd
    bcd = BCD::getInstance(bb);

    // set symbol
    {
        const auto& symbolRange = bcd.symbolRange();
        if (symbolRange.size() != 1) ERROR();
        const auto& range = symbolRange.at(0);
        auto bb2 = bb.range(range.offset, range.size);
        symbol = Symbol::getInstance(bb2);    
    }

    // for debug
    //dumpTable("se", symbol.seTable);

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
    out.nest();
    out.println("DIRECTORY");

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
    out.nest();
    out.println("%s %s = BEGIN", own.stamp.toString(), own.moduleId.toValue());

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
    if (s)  bitspec += std_sprintf(":%d..%d", a.bd + 0, a.bd + s - 1);
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
        auto vf = printType(context, typeSei, noSub);
        printDefaultValue(context, sei, vf);
    } else {
        auto typeSei = id.idType;
        if (id.immutable && !id.constant) {
            auto xferMode = SymbolOps::xferMode(symbol, typeSei);
            if (xferMode == TransferMode::NONE || xferMode == TransferMode::PROCESS) out.print("READONLY ");
        }
        auto vf = printType(context, typeSei, noSub);
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
                    if (underTypeSei.value().toCONS().isSUBRANGE()) {
                        const auto& subrange = underTypeSei.value().toCONS().toSUBRANGE();
                        printTypedVal(context, sei.value().toID().idValue + subrange.origin, vf);
                    } else {
                        printTypedVal(context, sei.value().toID().idValue, vf);
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

    if (isei.isNull()) { // special for empty case
        out.print("[]");
    } else {
        out.println("[");
        bool first = true;
        for(; !isei.isNull(); isei = SymbolOps::nextSe(symbol, isei)) {
            if (first) first = false;
            else out.println(", ");
            if (md) getBitSpec(context, isei, bitspec);
            printSym(context, isei, bitspec);
            printDefaultValue(context, isei, getValFormat(context, isei.value().toID().idType));
        }
        out.print("]");    
    }
}

void printTypedVal(Context& context, uint16_t val, ValFormat vf) {
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
        break;
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
            const auto& c = id.idCtx.value();
            if (c.isINCLUDED()) {
                printHti(context, id.idCtx.value().toINCLUDED().module.value().moduleId);
                out.print(".");
            } else if (c.isSIMPLE()) {
                putCurrentModuleDot(context);
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
            {
                auto enumarated = cons.toENUMERATED();
                if (enumarated.machineDep) out.print("MACHINE DEPENDENT ");
                bool first = true;
                uint16_t v = 0;
                out.nest();
                out.println("{");
                for(auto isei = SymbolOps::firstCtxSe(symbol, enumarated.valueCtx);
                    !isei.isNull();
                    isei = SymbolOps::nextSe(symbol, isei)
                ) {
                    if (first) first = false;
                    else out.println(", ");
                    const auto& id = isei.value().toID();
                    if (enumarated.machineDep || context.showBits) {
                        auto hti = id.hash;
                        auto sv = id.idValue;
                        if (!hti.isNull()) printSei(context, isei);
                        if (sv != v || hti.isNull() || context.showBits) out.print("(%u)", sv);
                        v = sv + 1;                        
                    } else printSei(context, isei);
                }
//                out.println();
                out.unnest();
                out.print("}");
            }
                break;
            case TypeClass::RECORD:
                {
                    auto record = cons.toRECORD();
                    if (record.fieldCtx.value().level != ContextLevel::LZ) {
                        auto fctx = record.fieldCtx;
                        for(auto [key, value]: symbol.bodyTable) {
                            const auto& entry = *value;
                            if (entry.isCALLABLE()) {
                                const auto& callable = entry.toCALLABLE();
                                if (entry.localCtx == fctx) {
                                    printSei(context, callable.id);
                                    break;
                                }
                            }
                        }
                    } else {
                        auto dp = context.defaultPublic;
                        if (context.defaultPublic && record.hints.privateFields) {
                            out.print("PRIVATE ");
                            context.defaultPublic = false;
                        }
                        if (record.monitored) out.print("MONITORED ");
                        if (record.machindDep) out.print("MACHINE DEPENDENT ");
                        out.nest();
                        out.print("RECORD ");
                        printFieldCtx(context, record.fieldCtx, record.machindDep || context.showBits);
                        out.unnest();
                        context.defaultPublic = dp;
                    }
                }
                break;
            case TypeClass::REF:
                {
                    const auto& ref = cons.toREF();
                    if (ref.var) {
                        out.print(ref.readOnly ? "READONLY " : "VAR ");
                    } else {
                        if (ref.ordered) out.print("ORDERED ");
                        if (ref.basing) out.print("BASE ");
                        out.print("POINTER");
                        dosub();
                        auto& underType = SymbolOps::underType(symbol, ref.refType).value();
                        if (underType.toCONS().isBASIC()) {
                            const auto& basic = underType.toCONS().toBASIC();
                            if (basic.code == SEIndex::TYPE_ANY && !ref.readOnly) break;
                        }
                        out.print(" TO ");
                        if (ref.readOnly) out.print("READONLY ");
                    }
                    printType(context, ref.refType, noSub);
                }
                break;
            case TypeClass::ARRAY:
            {
                const auto& array = cons.toARRAY();
                if (array.packed) out.print("PACKED ");
                out.print("ARRAY ");
                printType(context, array.indexType, noSub);
                out.print(" OF ");
                printType(context, array.componentType, noSub);
            }
                break;
            case TypeClass::ARRAYDESC:
            {
                const auto& arraydesc = cons.toARRAYDESC();
                out.print("DESCRIPTOR FOR ");
                if (arraydesc.readOnly) out.print("READONLY ");
                printType(context, arraydesc.describedType, noSub);
            }
                break;
            case TypeClass::TRANSFER:
            {
                const auto& transfer = cons.toTRANSFER();
                putModeName(context, transfer.mode);
                if (!transfer.typeIn.isNull()) {
                    out.nest();
                    out.print(" ");
                    outArgType(context, transfer.typeIn);
                    out.unnest();
                }
                if (!transfer.typeOut.isNull()) {
                    out.nest();
                    out.println();
                    out.print("RETURNS ");
                    out.nest();
                    outArgType(context, transfer.typeOut);
                    out.unnest();
                    out.unnest();
                }
            }
                break;
            case TypeClass::UNION:
            {
                const auto& union_ = cons.toUNION();
                auto tagSei = union_.tagSei;
                out.print("SELECT ");
                if (!union_.controlled) out.print(union_.overlaid ? "OVERLAID " : "COMPUTED ");
                else {
                    printSei(context, tagSei);
                    if (union_.machineDep || context.showBits) {
                        std::string bitspec;
                        getBitSpec(context, tagSei, bitspec);
                        out.print(bitspec);
                    } else out.print(": ");
                }
                const auto tagType = tagSei.value().toID().idType;
                if (tagSei.value().toID().public_ != context.defaultPublic)
                    out.print(context.defaultPublic ? "PRIVATE " : "PUBLIC ");
                if (tagType.value().isID()) printType(context, tagType, noSub);
                if (tagType.value().isCONS()) out.print("*");
                out.nest();
                out.println(" FROM");
                SEIndex temp;
                for(auto isei = SymbolOps::firstCtxSe(symbol, union_.caseCtx);
                    !isei.isNull();
                    isei = temp) {
                    printSei(context, isei);
                    SEIndex varRec = SymbolOps::underType(symbol, SymbolOps::toSei(symbol, isei.value().toID().idInfo));
                    for(temp = SymbolOps::nextSe(symbol, isei);
                        !temp.isNull() && SymbolOps::toSei(symbol, temp.value().toID().idInfo) == isei;
                        temp = SymbolOps::nextSe(symbol, temp)) {
                        out.print(", ");
                        printSei(context, temp);
                    }
                    out.print(" => ");
                    out.nest();
                    printFieldCtx(context, varRec.value().toCONS().toRECORD().fieldCtx, union_.machineDep || context.showBits);
                    out.unnest();
                    out.println(",");
                }
//                out.println();
                out.print("ENDCASE");
                out.unnest();
            }
                break;
            case TypeClass::RELATIVE:
            {
                const auto& relative = cons.toRELATIVE();
                if (!relative.baseType.isNull()) printType(context, relative.baseType, noSub);
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
                printType(context, tagType, noSub);
                out.print(" OF ");
                printType(context, sequnce.componentType, noSub);
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
                printType(context, rangeType, noSub);
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
    auto [extType, tree] = SymbolOps::findExtension(context.symbol, sei);
    if (extType != ExtensionType::DEFAULT) return;
    context.out.print(" = ");
    printTreeLink(context, tree, vf, false);
}

void putWordSeq(Context& context, const std::vector<uint16_t>& vector) {
    auto& out = context.out;

    std::string string;
    for(auto e: vector) {
        string += std_sprintf("%04X", e);
    }
    out.print("(%d)[%s]", vector.size(), string);
}

void printTreeLink(Context& context, TreeLink tree, ValFormat vf, int recur, bool sonOfDot) {
    (void)vf;
    const auto& symbol = context.symbol;
    auto& out = context.out;

    if (tree.isNull()) return;
    if (30 < recur) ERROR()

    switch(tree.tag) {
        case TreeLink::Tag::SUBTREE:
            {
                const auto& subtree = tree.toSUBTREE();
                const auto& node = subtree.index.value();
                switch(node.name) {
                case NodeName::ALL:
                {
                    out.print("ALL[");
                    if (vf.isARRAY()) {
                        const auto& array = vf.toARRAY();
                        printTreeLink(
                            context, node.son[0], getValFormat(context, array.componentType), recur + 1);
                    } else {
                        if (node.nSons == 1) printTreeLink(context, node.son[0], vf, recur + 1);
                        else ERROR()
                    }
                    out.print("]");
                }
                    break;
                case NodeName::MWCONST:
                case NodeName::CASE:
                case NodeName::LOOPHOLE:
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    break;
                case NodeName::NIL:
                    out.print("NIL");
                    break;
                case NodeName::VOID:
                    out.print("NULL");
                    break;
                case NodeName::DOT:
                case NodeName::CDOT:
                    if(node.nSons != 2) ERROR()
                    printTreeLink(context, node.son[0], ValFormat::getOTHER(), recur + 1, true);
                    out.print(".");
                    printTreeLink(context, node.son[1], ValFormat::getOTHER(), recur + 1, true);
                    break;
                case NodeName::FIRST:
                    out.print("FIRST[");
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::LAST:
                    out.print("LAST[");
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::SIZE:
                    out.print("SIZE[");
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::LENGTHEN:
                    out.print("LONG[");
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::CONSTRUCT:
                    out.print("[");
                    if (node.nSons == 2) printTreeLink(context, node.son[1], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::UNION:
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    out.print("[");
                    printTreeLink(context, node.son[1], vf, recur + 1);
                    out.print("]");
                    break;
                case NodeName::LIST:
                    for(int j = 0; j < node.nSons - 1; j++) {
                        printTreeLink(context, node.son[j], ValFormat::getOTHER(), recur + 1);
                    }
                    if (node.nSons) printTreeLink(context, node.son[node.nSons - 1], ValFormat::getOTHER(), recur + 1);
                    break;
                case NodeName::LONG_TC:
                    out.print("LONG ");
                    printTreeLink(context, node.son[0], vf, recur + 1);
                    break;
                case NodeName::UPARROW:
                    {
                        printTreeLink(context, node.son[0], vf, recur + 1);
                        out.print("### UPARRWO ###"); // FIXME
                    }
                    break;
                default:
                    out.print("(complicated expression  %s)", toString(node.name));
                }
            }
            break;
        case TreeLink::Tag::HASH:
            printHti(context, tree.toHASH().index);
            break;
        case TreeLink::Tag::SYMBOL:
            if (!sonOfDot && tree.toSYMBOL().index.value().toID().idCtx == symbol.outerCtx)
                putCurrentModuleDot(context);
            printSei(context, tree.toSYMBOL().index);
            break;
        case TreeLink::Tag::LITERAL:
            {
                const auto index = tree.toLITERAL().index;
                switch(index.tag) {
                case LitRecord::Tag::WORD:
                {
                    const auto& word = index.toWORD().index.value();
                    switch(word.tag) {
                    case LTRecord::Tag::SHORT:
                        printTypedVal(context, word.toSHORT().value, vf);
                        break;
                    case LTRecord::Tag::LONG:
                    {
                        const auto& long_ = word.toLONG();
                        if (long_.length == 2) {
                            uint32_t longValue = long_.value[1] << 16| long_.value[0];
                            bool loophole = false;
                            switch(vf.tag) {
                            case ValFormat::Tag::SIGNED:
                                out.print("%d", longValue);
                                break;
                            case ValFormat::Tag::UNSIGNED:
                                out.print("%u", longValue);
                                break;
                            case ValFormat::Tag::TRANSFER:
                            case ValFormat::Tag::REF:
                                if (longValue == 0) out.print("NIL");
                                else loophole = true;
                                break;
                            default:
                                loophole = true;
                                break;
                            }
                            if (loophole)  out.print("LOOPHOLE[%u]", longValue);
                        } else putWordSeq(context, long_.value);
                    }
                        break;
                    default: ERROR()
                    }
                }
                    break;
                case LitRecord::Tag::STRING:
                {
                    const auto& string = index.toSTRING();
                    out.print("(LONG STRING %d)", string.index);
                }
                    break;
                default: ERROR()
                }
            }
            break;
        default:
            ERROR()
    }
}

}

