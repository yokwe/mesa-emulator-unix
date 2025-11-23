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

#include "../util/Util.h"
static const Logger logger(__FILE__); 

#include "../util/ByteBuffer.h"

#include "../mesa/Pilot.h"

#include "Symbols.h"
#include "BCD.h"

BCD BCD::getInstance(ByteBuffer& bb) {
    NameRecord::clear();
    FTIndex::clear();
    SGIndex::clear();
    ENIndex::clear();
    MTIndex::clear();
    
    BCD bcd;

    bcd.read(bb);

    bcd.buildSSTable(bb);
    bcd.buildFTTable(bb);
    bcd.buildSGTable(bb);
    bcd.buildENTable(bb);
    bcd.buildMTTable(bb);

    bcd.setValue();

    bcd.setSymbolOffset(bb);

    return bcd;
}

void BCD::setValue() const {
    NameRecord::setValue(ssTable);
    FTIndex::setValue(ftTable);
    SGIndex::setValue(sgTable);
    ENIndex::setValue(enTable);
    MTIndex::setValue(mtTable);
}

ByteBuffer& BCD::read(ByteBuffer& bb) {
    bb.rewind();
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

void BCD::setSymbolOffset(ByteBuffer& bb) {
    symbolOffset = 0;
    if (nModules == 0) {
        // try Symbols::ALTO_BIAS
        auto offset = Symbols::ALTO_BIAS * Environment::bytesPerPage;;
        bb.position(offset);
        auto word = bb.get16();
        if (word == Symbols::VersionID) symbolOffset = offset;
        // sanity check
        if (symbolOffset == 0) ERROR()
    } else if (nModules == 1) {
         // try symbol segemtn
        int count = 0;
        for(const auto& e: sgTable) {
            const SGRecord& sgRecord = *e.second;
            if (!sgRecord.file.isSelf()) continue;
            
            if (sgRecord.segClass == SGRecord::SegClass::SYMBOLS) {
                auto offset = (sgRecord.base - Symbols::ALTO_BIAS) * Environment::bytesPerPage;
                bb.position(offset);
                auto word = bb.get16();
                if (word == Symbols::VersionID) symbolOffset = offset;
                count++;
            }
        }
        // sanity check
        if (count != 1) {
            logger.error("Unexpected count");
            logger.error("sgOffset %d", sgOffset);
            logger.error("sgListm  %d", sgLimit);
            logger.error("count    %d", count);
            ERROR()
        }
    } else {
        // more than one module
    }
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

	logger.info("symbolOffset   %5d", symbolOffset);

}
void BCD::dumpTable() {
    // for(const auto& e: ssTable) {
    //     auto key = e.first;
    //     auto value = e.second;
    //     logger.info("%-8s  %s", std_sprintf("%s-%d", "ss", key), value);
    // }
    {
        int fileIndex = 0;
        for(const auto& e: ftTable) {
            auto key = e.first;
            auto& value = *e.second;
            logger.info("%-8s  %5d  %s", std_sprintf("%s-%d", "ft", key), fileIndex++, value.toString());
        }
    }
    for(const auto& e: sgTable) {
        auto key = e.first;
        auto& value = *e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", "sg", key), value.toString());
    }
    for(const auto& e: enTable) {
        auto key = e.first;
        auto& value = *e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", "en", key), value.toString());
    }
    for(const auto& e: mtTable) {
        auto key = e.first;
        auto& value = *e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", "mt", key), value.toString());
    }

    logger.info("ssTable  %d", ssTable.size());
    logger.info("ftTable  %d", ftTable.size());
    logger.info("sgTable  %d", sgTable.size());
    logger.info("enTable  %d", enTable.size());
    logger.info("mtTable  %d", mtTable.size());
}
void BCD::dumpIndex() {
//    NameRecord::dump();
    FTIndex::dump();
    SGIndex::dump();
    ENIndex::dump();
    MTIndex::dump();

    logger.info("NameRecord indexSet  %d", NameRecord::indexSet.size());
    logger.info("FTIndex    indexSet  %d", FTIndex::indexSet.size());
    logger.info("SGIndex    indexSet  %d", SGIndex::indexSet.size());
    logger.info("ENIndex    indexSet  %d", ENIndex::indexSet.size());
    logger.info("MTIndex    indexSet  %d", MTIndex::indexSet.size());
}

uint16_t BCD::getIndex(int pos, int offset, int limit) {
    uint16_t index = ((pos + 1) / 2) -  offset;
    if (index < 0 || limit < index) {
        logger.error("Unexpected index");
        logger.error("  pos     %5d", pos);
        logger.error("  offset  %5d", offset);
        logger.error("  limit   %5d", limit);
        logger.error("  index   %5d", index);
        ERROR()
    }
    return index;
}

void BCD::buildSSTable(ByteBuffer& bb) {
	int   offset = ssOffset;
	int   limit  = ssLimit;
    auto& table  = ssTable;

	if (limit == 0) return;

    bb.position((offset + 2) * 2);
    bb.get8();
    for(;;) {
        auto pos = bb.position();
        uint16_t index = getIndex(pos, offset, limit);
        if (limit <= index) break;

        std::string* value = new std::string;
        auto length = bb.get8();
        for(int i = 0; i < length; i++) {
            *value += bb.get8();
        }
        int ssIndex = pos - (offset * 2) - 3;

        if (table.contains(ssIndex)) {
            // not expect this
            logger.error("Duplicate key");
            logger.error("  index   %d", index);
            ERROR()
        } else {
            table[ssIndex] = value;
        }
   }
}

template<class T>
static void buildTable(ByteBuffer& bb, int offset, int limit_, std::map<uint16_t, T*>& table) {
    int base  = offset * 2;
    int limit = base + limit_ * 2;
    bb.position(base);
    for(;;) {
        if (limit <= bb.position()) break;
        int index = BCD::getIndex(bb.position(), offset, limit);
        T* value = new T;
        value->read(bb);
        table[index] = value;
    }
	// sanity check
	if (bb.position() != limit) {
		logger.error("Unexpected length");
		logger.error("  pos        %5d", bb.position());
		logger.error("  base       %5d", base);
		logger.error("  limit      %5d", limit);
		logger.error("  index      %5d", index);
	}
}

void BCD::buildFTTable(ByteBuffer& bb) {
    buildTable<FTRecord>(bb, ftOffset, ftLimit, ftTable);
}
void BCD::buildSGTable(ByteBuffer& bb) {
    buildTable<SGRecord>(bb, sgOffset, sgLimit, sgTable);
}
void BCD::buildENTable(ByteBuffer& bb) {
    buildTable<ENRecord>(bb, enOffset, enLimit, enTable);
}
void BCD::buildMTTable(ByteBuffer& bb) {
    buildTable<MTRecord>(bb, mtOffset, mtLimit, mtTable);
}


ByteBuffer& SGRecord::read(ByteBuffer& bb) {
    uint16_t word;

    bb.read(file, base, word);
    pages      = bitField(word, 0, 7);
    extraPages = bitField(word, 8, 13);
    segClass   = (SGRecord::SegClass)bitField(word, 14, 15);

    return bb;
}
// CODE, SYMBOLS, AC_MAP, OTHER,
static std::map<SGRecord::SegClass, std::string> segClassMap = {
    {SGRecord::SegClass::CODE,    "CODE"},
    {SGRecord::SegClass::SYMBOLS, "SYMBOLS"},
    {SGRecord::SegClass::AC_MAP,  "AC_MAP"},
    {SGRecord::SegClass::OTHER,   "OTHER"},
};
std::string SGRecord::toString() const {
    return std_sprintf("%s#%d#%d#%d#%s", file.toString(), base, pages, extraPages, segClassMap[segClass]);
}


ByteBuffer& ENRecord::read(ByteBuffer& bb) {
    uint16_t nEntries;
    bb.read(nEntries);
    initialPC.reserve(nEntries);
    for(int i = 0; i < nEntries; i++) {
        initialPC.push_back(bb.get16());
    }
    return bb;
}
std::string ENRecord::toString() const {
    std::string tmp;
    for(size_t i = 0; i < initialPC.size(); i++) {
        tmp += std_sprintf(" %d", initialPC[i]);
    }
    return std_sprintf("[%s]", tmp.substr(1));
}


ByteBuffer& MTRecord::read(ByteBuffer& bb) {
    uint16_t u8;
    bb.read(name, file, config, code, sseg, links, u8, frameSize, entries, atoms);

    linkLoc        = (LinkLocation)bitField(u8,  0, 1);
    namedInstance  = (bool)bitField(u8,  2,  2);
    initial        = (bool)bitField(u8,  3,  3);
    boundsChecks   = (bool)bitField(u8,  4,  4);
    nilChecks      = (bool)bitField(u8,  5,  5);
    tableCompiled  = (bool)bitField(u8,  6,  6);
    residentFrame  = (bool)bitField(u8,  7,  7);
    crossJumped    = (bool)bitField(u8,  8,  8);
    packageable    = (bool)bitField(u8,  9,  9);
    packed         = (bool)bitField(u8, 10, 10);
    linkspace      = (bool)bitField(u8, 11, 11);
    spare0         = (bool)bitField(u8, 12, 12);
    spare1         = (bool)bitField(u8, 13, 13);
    spare2         = (bool)bitField(u8, 14, 14);
    spare3         = (bool)bitField(u8, 15, 15);

    return bb;
}
std::string MTRecord::toString() const {
    return std_sprintf("[%s  %s  %s  %s]", name.toString(), file.toString(), code.toString(), sseg.toString());
}
