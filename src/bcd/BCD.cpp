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

#include "BCD.h"

ByteBuffer& BCD::read(ByteBuffer& bb) {
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
}

uint16_t BCD::getIndex(int pos, int offset, int limit) {
    uint16_t index = (pos / 2) -  offset;
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

void BCD::readTableSS(ByteBuffer& bb) {
	int offset = ssOffset;
	int limit  = ssLimit;

	if (limit == 0) return;

    bb.position((offset + 2) * 2);
    bb.get8();
    for(;;) {
        auto pos = bb.position();
        uint16_t index = getIndex(pos, offset, limit);
        if (limit <= index) break;

        std::string value;
        auto length = bb.get8();
        for(int i = 0; i < length; i++) {
            value += bb.get8();
        }
        int ssIndex = pos - (offset * 2) - 3;
        std::string* p = new std::string;
        *p = value;
        NameRecord::addValue(ssIndex, p);
    }
}

template<class R, class I>
static void readTable(ByteBuffer& bb, int offset, int limit) {
    bb.position(offset * 2);
    for(;;) {
        uint16_t index = BCD::getIndex(bb.position(), offset, limit);
        if (limit <= index) break;

        R entry;
        entry.read(bb);
        I::addValue(index, new R(entry));
    }
}

void BCD::readTableFT(ByteBuffer& bb) {
    readTable<FTRecord, FTIndex>(bb, ftOffset, ftLimit);
}
void BCD::readTableSG(ByteBuffer& bb) {
    readTable<SGRecord, SGIndex>(bb, sgOffset, sgLimit);
}
void BCD::readTableEN(ByteBuffer& bb) {
    readTable<ENRecord, ENIndex>(bb, enOffset, enLimit);
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
std::string SGRecord::toString() {
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
std::string ENRecord::toString() {
    std::string tmp;
    for(size_t i = 0; i < initialPC.size(); i++) {
        tmp += std_sprintf(" %d", initialPC[i]);
    }
    return std_sprintf("[%s]", tmp.substr(1));
}
