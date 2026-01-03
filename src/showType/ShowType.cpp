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

#include "../bcdFile/MDIndex.h"
#include "../bcdFile/MDRecord.h"
#include "../bcdFile/CTXRecord.h"
#include "../bcdFile/SERecord.h"

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

    for(auto sei = symbol.outerCtx.value().seList; !sei.isNull(); sei = symbol.nextSei(sei)) {
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


void printSym(Context& context, SEIndex sei, const std::string& colongstring) {
    (void)sei, (void)colongstring;
    auto& out = context.out;
    auto& symbol = context.symbol;

    out.print("<< %s >>", __FUNCTION__);
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
        out.print("<< %s NO-TYPE >>", __FUNCTION__); // FIXME
    }

    context.defaultPublic = savePublic;
}

ValFormat printType(Context& context, SEIndex, std::function<void()> dosub) {
    (void)dosub;
    context.out.print("<< %s >>", __FUNCTION__); // FIXME
    return ValFormat::getOTHER();
}

void printDefaultValue(Context& context, SEIndex sei, ValFormat vf) {
    (void)sei, (void)vf;
    context.out.print("<< %s >>", __FUNCTION__); // FIXME
}

}

