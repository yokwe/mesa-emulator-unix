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
// BCD.cpp
//

#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/ByteBuffer.h"

#include "../mesa/Pilot.h"

#include "BCD.h"
#include "Symbol.h"

#include "Type.h"
#include "ENIndex.h"
#include "ENRecord.h"
#include "MTIndex.h"
#include "MTRecord.h"
#include "FTIndex.h"
#include "FTRecord.h"
#include "SGIndex.h"
#include "SGRecord.h"

static void readTableSS(MesaByteBuffer& baseBB, uint32_t offset, uint32_t limit, std::map<uint16_t, std::string*>& table) {
    if (limit == 0) return;
    auto bb = baseBB.range(offset, limit);

    bb.pos(2);
    bb.get8();
    for(;;) {
        auto index = bb.pos();
        if (limit <= index) break;
        auto bytePos = bb.bytePos();

        std::string* value = new std::string;
        auto length = bb.get8();
        for(int i = 0; i < length; i++) {
            *value += bb.get8();
        }
        int ssIndex = bytePos - 3;
        table[ssIndex] = value;
   }
}
template<class T>
static void readTable(MesaByteBuffer& baseBB, uint32_t offset, uint32_t limit, std::map<uint16_t, T*>& table) {
    if (limit == 0) return;
    auto bb = baseBB.range(offset, limit);

    bb.pos(0);
    for(;;) {
        auto index = bb.pos();
        if (limit <= index) break;

        T* value = new T;
        value->read(bb);
        table[index] = value;
    }
	// sanity check
	if (bb.pos() != limit) {
		logger.error("Unexpected length");
		logger.error("  pos        %5d", bb.pos());
		logger.error("  limit      %5d", limit);
		logger.error("  index      %5d", index);
	}
}

BCD BCD::getInstance(MesaByteBuffer& bb) {
    // sanity check
    checkVersionIdent(bb);

    BCD bcd;

    bcd.read(bb);

    // read table
    readTableSS(bb, bcd.ssOffset, bcd.ssLimit, bcd.ssTable);    
    readTable<FTRecord>(bb, bcd.ftOffset, bcd.ftLimit, bcd.ftTable);
    readTable<SGRecord>(bb, bcd.sgOffset, bcd.sgLimit, bcd.sgTable);
    readTable<ENRecord>(bb, bcd.enOffset, bcd.enLimit, bcd.enTable);
    readTable<MTRecord>(bb, bcd.mtOffset, bcd.mtLimit, bcd.mtTable);

    // set index value
    NameRecord::setValue(bcd.ssTable);
    FTIndex::setValue(bcd.ftTable);
    SGIndex::setValue(bcd.sgTable);
    ENIndex::setValue(bcd.enTable);
    MTIndex::setValue(bcd.mtTable);

    // build bcd.mySymbolRange
    {
        if (bcd.sgTable.empty()) {
            uint32_t offset = Symbol::ALTO_BIAS * Environment::wordsPerPage;
            uint32_t size = bb.size() - offset;

            // sanity check
            bb.pos(offset);
            Symbol::checkVersionIdent(bb);

            bcd.mySymbolRange.emplace_back(offset, size);
            logger.info("mySymbolRange  A  %5d  %5d", offset, size);
        } else {
            for(const auto& e: bcd.sgTable) {
                SGRecord& sgRecord  = *e.second;
                if (!sgRecord.file.isSelf()) continue;
                if (sgRecord.segClass != SGRecord::SegClass::SYMBOLS) continue;

                uint32_t offset = (sgRecord.base - Symbol::ALTO_BIAS) * Environment::wordsPerPage;
                uint32_t size   = sgRecord.pages * Environment::wordsPerPage;

                // sanity check
                bb.pos(offset);
                Symbol::checkVersionIdent(bb);
    
                bcd.mySymbolRange.emplace_back(offset, size);
                logger.info("mySymbolRange  B  %5d  %5d", offset, size);
            }
        }
    }

    return bcd;
}

void BCD::checkVersionIdent(MesaByteBuffer &bb) {
    auto oldPos = bb.pos();
    auto word = bb.get16();
    bb.pos(oldPos);
    if (word == BCD::VersionID) return;
    logger.error("Unexpected version  %d", word);
    ERROR()
}

ByteBuffer& BCD::read(ByteBuffer& bb) {
    bb.pos(0);
    bb.read(versionIdent);
    // sanity check
    if (versionIdent != VersionID) ERROR();
    bb.read(version, creator, sourceFile, unpackagedFile, nConfigs, nModules, nImports, nExports);

    uint16_t flags;
    bb.read(flags);
    nPages        = bitField(flags, 0, 7);
    definitions   = bitField(flags, 8);
    repackaged    = bitField(flags, 9);
    typeExported  = bitField(flags, 10);
    tableCompiled = bitField(flags, 11);
    spare4        = bitField(flags, 12, 15);

    bb.read(firstDummy, nDummies);

    #undef BCD_TABLE
    #define BCD_TABLE(prefix) bb.read(prefix##Offset, prefix##Limit);

    BCD_TABLE(ss)  // string table
    BCD_TABLE(ct)  // config table
    BCD_TABLE(mt)  // module table
    BCD_TABLE(imp) // import table
    BCD_TABLE(exp) // export table
    BCD_TABLE(en)  // entry table
    BCD_TABLE(sg)  // segment table
    BCD_TABLE(ft)  // file table
    BCD_TABLE(sp)  // space table
    BCD_TABLE(nt)  // name table
    BCD_TABLE(typ) // type table
    BCD_TABLE(tm)  // type map table
    BCD_TABLE(fp)  // frame pack table
    BCD_TABLE(lf)  // link fragment table
    BCD_TABLE(at)  // atom table
    BCD_TABLE(ap)  // atom print table

    return bb;
}

void BCD::dump() {
	logger.info("versionIdent   %5d", versionIdent);
	logger.info("version            %s", version.toString());
	logger.info("creator            %s", creator.toString());
	logger.info("sourceFile         %s", sourceFile.toString());
	logger.info("unpackagedFile     %s", unpackagedFile.toString());

	logger.info("nConfigs       %5d", nConfigs);
	logger.info("nModules       %5d", nModules);
	logger.info("nImports       %5d", nImports);
	logger.info("nExports       %5d", nExports);
	logger.info("nPages         %5d", nPages);
	logger.info("flags              %s%s%s%s", definitions ? "definitions " : "", repackaged ? "repackaged " : "", typeExported ? "typeExported" : "", tableCompiled ? "tableCompiled " : "");
	logger.info("firstDummy     %5d", firstDummy);
	logger.info("nDummies       %5d", nDummies);

    #undef BCD_TABLE
    #define BCD_TABLE(prefix) logger.info("%-3s            %5d  %5d", #prefix, prefix##Offset, prefix##Limit);

    BCD_TABLE(ss)  // string table
    BCD_TABLE(ct)  // config table
    BCD_TABLE(mt)  // module table
    BCD_TABLE(imp) // import table
    BCD_TABLE(exp) // export table
    BCD_TABLE(en)  // entry table
    BCD_TABLE(sg)  // segment table
    BCD_TABLE(ft)  // file table
    BCD_TABLE(sp)  // space table
    BCD_TABLE(nt)  // name table
    BCD_TABLE(typ) // type table
    BCD_TABLE(tm)  // type map table
    BCD_TABLE(fp)  // frame pack table
    BCD_TABLE(lf)  // link fragment table
    BCD_TABLE(at)  // atom table
    BCD_TABLE(ap)  // atom print table

    logger.info("ssTable        %5d", ssTable.size());
    logger.info("ftTable        %5d", ftTable.size());
    logger.info("sgTable        %5d", sgTable.size());
    logger.info("enTable        %5d", enTable.size());
    logger.info("mtTable        %5d", mtTable.size());
	logger.info("nDummies       %5d", nDummies);

    logger.info("symbolRange    %5d", this->mySymbolRange.size());
}

template <class T>
static void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
    for(const auto& [key, value]: map) {
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
    }
}
void BCD::dumpTable() {
    // ::dumpTable("ft", ftTable);
    ::dumpTable("sg", sgTable);
    // ::dumpTable("en", enTable);
    ::dumpTable("mt", mtTable);
}
