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
// PrntSymbol.cpp
//

#include <filesystem>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/StringPrinter.h"

#include "Type.h"
#include "BCDFile.h"
#include "BCD.h"
#include "Symbol.h"

#include "MDIndex.h"
#include "MDRecord.h"
#include "CTXRecord.h"
#include "SERecord.h"
#include "BodyRecord.h"
#include "EXTRecord.h"


#include "PrintSymbol.h"

namespace print_symbol {

template <class T>
static void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
    for (auto const& [key, value] : map) {
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
    }
}
    
Context::Context(const std::string& outDir, const std::string& bcdPath_) : bcdPath(bcdPath_), bcdFile(bcdPath) {
    auto bb = bcdFile.mesaByteBuffer();
    bcd = BCD::getInstance(bb);

    auto range = bcd.symbolRange().at(0);
    symbol = Symbol::getInstance(bb.range(range.offset, range.size));

    symbol.dump();
    dumpTable("se", symbol.seTable);

    std::filesystem::path inputPath(bcdPath);
    std::filesystem::path outputPath = outDir;
    outputPath += inputPath.stem();
    outputPath += ".symbol";

    logger.info("otputPath  %s", outputPath.string());

    outPath = outputPath;
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

	// Print Directory
	{
        if (!symbol.mdTable.empty()) {
            out.println("DIRECTORY");
            out.nest();

            auto begin = symbol.mdTable.cbegin();
            auto end = symbol.mdTable.cend();

            for(auto i = begin; i != end; i++) {
                auto index = i->first;
                if (index == MDIndex::MD_OWN) continue;
                auto& md = *(i->second);
                out.println("%-30s %s%s", md.moduleId.toValue(), md.stamp.toString(), isLast(i, end) ? ";" : ",");
            }

            out.unnest();
            out.println();
		}
	}
}

void printModule(Context& context) {
    auto& symbol = context.symbol;
    auto& out = context.out;

    auto& own = *symbol.mdTable[MDIndex::MD_OWN];
    out.println("%s %s = BEGIN", own.stamp.toString(), own.moduleId.toValue());
    out.nest();

    for(auto sei = symbol.outerCtx.value().seList; !sei.isNull(); sei = symbol.nextSei(sei)) {
        printSymbol(context, sei, ": ");
        out.println(";");
    }

    out.unnest();
    out.println("END.");
}

void printSymbol(Context& context, SEIndex sei, const std::string& colonString) {
    auto& symbol = context.symbol;
    auto& out = context.out;
    (void)symbol; (void)out;

    auto id = sei.value().toID();
    out.print("%s%s", id.hash.toValue(), colonString);

    if (!id.public_) out.print("PRIVATE ");
    if (sei.isType()) {
        out.print("TYPE");
        auto typeSei = symbol.toSEIndex(id.idInfo);
        const auto& seRecord = typeSei.value();
        if (seRecord.isCONS()) {
            if (!seRecord.toCONS().isOPAQUE()) {
                out.print(" = ");
            }
        } else {
            out.print(" = ");
        }

		ValFormat vf = printType(context, typeSei, {});
		printDefaultValue(context, sei, vf);
    } else {
        // FIXME
        auto typeSei = id.idType;
		if (id.immutable && (!id.constant)) {
			switch (symbol.xferMode(typeSei)) {
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
                auto bti = symbol.toBTIndex(id.idInfo);
                if (!bti.isNull()) {
                    const auto& bt = bti.value();
                    auto mode = vf.toTRANSFER().mode;
                    if (mode == TransferMode::SIGNAL || mode == TransferMode::ERROR_) {
                        out.print("CODE");
					} else if (bt.toCALLABLE().inline_) {
                        out.print("INLINE");
					} else {
                        out.print("BODY");
					}
                } else {
                    if (id.extended) {
                        out.print("OPCODE");
                    } else {
                        printTypedVal(context, id.idValue, vf);
                    }
                }
            } else {
				if (id.extended) {
                    auto exi = symbol.toEXTIndex(sei);
					printTreeLink(context, exi.value().tree, vf, 0, false);
				} else {
					const auto& underType = symbol.underType(typeSei).value();
					if (underType.toCONS().tag == TypeClass::SUBRANGE) {
						printTypedVal(context, id.idValue + underType.toCONS().toSUBRANGE().origin, vf);
					} else {
						printTypedVal(context, id.idValue, vf);
					}
				}

            }
        }
    }
}

ValFormat printType(Context& context, SEIndex sei, std::function<void()> dosub) {
    auto& symbol = context.symbol;
    auto& out = context.out;
//    logger.info("printType  %s", sei.value().toString());

    // FIXME
    ValFormat vf = getValFormat(context, sei);
    const auto& se = sei.value();
    switch(se.tag) {
    case SERecord::Tag::ID:
    {
        const auto& id = se.toID();
        for (;;) {
            const SEIndex next = symbol.typeLink(sei);
            if (next.isNull())
                break;
            if (sei.value().tag == SERecord::Tag::ID) {
                out.print("%s ", id.hash.toValue());
            }
            sei = next;
        }
        if (!id.idCtx.isStandardContext()) {
            const CTXRecord& c = id.idCtx.value();
            switch (c.tag) {
            case CTXRecord::Tag::INCLUDED:
                out.print("%s.", c.toINCLUDED().module.value().moduleId.toValue());
                break;
            case CTXRecord::Tag::SIMPLE:
                /* putCurrentModuleDot(out); */
                break;
            default:
                break;
            }
        }
        out.print(id.hash.toValue());
        if (dosub) dosub();
    }
        break;
    case SERecord::Tag::CONS:
        // FIXME
        out.print("-- printType CONS -- ");
        break;
    default:
        ERROR()
    }


    return vf;
}

ValFormat getValFormat(Context& context, SEIndex tsei) {
    (void)context; (void)tsei;
    auto& symbol = context.symbol;

    const auto& t = tsei.value();
    switch(t.tag) {
    case SERecord::Tag::ID:
        return getValFormat(context, symbol.underType(tsei));
    case SERecord::Tag::CONS:
    {
        const auto& cons = t.toCONS();
        switch(cons.tag) {
        case TypeClass::BASIC:
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
            break;
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
                auto vf = getValFormat(context, subrange.rangeType);
                if (vf.tag == ValFormat::Tag::SIGNED && subrange.origin == 0) vf = ValFormat::getUNSIGNED();
                return vf;
            }
            break;
        case TypeClass::LONG:
            return getValFormat(context, cons.toLONG().rangeType);
        case TypeClass::REF:
            return ValFormat::getREF();
        default:
            return ValFormat::getOTHER();
        }
    }
    default:
        ERROR()
    }
}

void printDefaultValue(Context& context, SEIndex sei, ValFormat vf) {
    (void)context; (void)sei; (void)vf;
    // FIXME
    auto& out = context.out;
    out.print("-- %s --", __FUNCTION__);
}

void printTypedVal(Context& context, uint16_t value, ValFormat vf) {
    (void)context; (void)value; (void)vf;
    // FIXME
    auto& out = context.out;
    out.print("-- %s --", __FUNCTION__);
}

void printTreeLink(Context& context, TreeLink tree, ValFormat vf, int recur, bool sonOfDot) {
    (void)context; (void)tree; (void)vf; (void) recur; (void)sonOfDot;
    // FIXME
    auto& out = context.out;
    out.print("-- %s --", __FUNCTION__);
}

}