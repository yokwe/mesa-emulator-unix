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

#include "BCDFile.h"
#include "BCD.h"
#include "Symbol.h"

#include "MDIndex.h"
#include "MDRecord.h"
#include "CTXRecord.h"
#include "SERecord.h"

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
    (void)sei; (void)colonString;
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
    }

    logger.info("sei  %s  %s", sei.toString(), sei.value().toString());
}

ValFormat printType(Context& context, SEIndex sei, std::function<void()> dosub) {
    (void)context; (void)sei; (void)dosub;
    // FIXME
    ValFormat ret;
    return ret;
}

ValFormat getValFormat(Context& context, SEIndex tsei) {
    (void)context; (void)tsei;
    // FIXME
    ValFormat ret;
    return ret;
}

void printDefaultValue(Context& context, SEIndex sei, ValFormat vf) {
    (void)context; (void)sei; (void)vf;
    // FIXME
}

}