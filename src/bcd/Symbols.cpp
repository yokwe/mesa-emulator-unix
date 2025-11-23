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
// Symbols.cpp
//


#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"

#include "HTRecord.h"
#include "MDRecord.h"
#include "CTXRecord.h"


#include "Symbols.h"


Symbols Symbols::getInstance(ByteBuffer &bb, int offset) {
	HTIndex::clear();

    Symbols symbols;

    bb.position(offset);
    symbols.read(bb);

    symbols.initializeHT(bb);
	symbols.initializeMD(bb);
	symbols.initializeCTX(bb);

	HTIndex::setValue(symbols.htTable);
	MDIndex::setValue(symbols.mdTable);
	CTXIndex::setValue(symbols.ctxTable);

    return symbols;
}

ByteBuffer& Symbols::read(ByteBuffer& bb) {
	symbolBase = bb.position();

	uint16_t u10;
	directoryCtx = new CTXIndex;
	importCtx    = new CTXIndex;
	outerCtx     = new CTXIndex;

	bb.read(versionIdent, version, creator, sourceVersion, u10, *importCtx, *outerCtx);
	bb.read(hvBlock, htBlock, ssBlock, outerPackBlock, innerPackBlock, constBlock);
	bb.read(seBlock, ctxBlock, mdBlock, bodyBlock, extBlock, treeBlock, litBlock, sLitBlock, epMapBlock, spareBlock);
	bb.read(fgRelPgBase, fgPgCount);

	definitionsFile = bitField(u10, 15);

	directoryCtx->index(bitField(u10, 1, 15));

	return bb;
}

void Symbols::dump() {
    logger.info("symbolBase      %5d  %5d", symbolBase, symbolBase / Environment::bytesPerPage);
    logger.info("versionIdent    %5d", versionIdent);
	logger.info("version            %s", version.toString());
	logger.info("creator            %s", creator.toString());
	logger.info("sourceVersion      %s", creator.toString());
	logger.info("definitionsFile    %s", definitionsFile ? "YES" : "NO");
	logger.info("directoryCtx       %s", directoryCtx->Index::toString());
	logger.info("importCtx          %s", importCtx->Index::toString());
	logger.info("outerCtx           %s", outerCtx->Index::toString());

	logger.info("hvBlock         %5d  %5d", hvBlock.offset, hvBlock.size);
	logger.info("htBlock         %5d  %5d", htBlock.offset, htBlock.size);
	logger.info("ssBlock         %5d  %5d", ssBlock.offset, ssBlock.size);
	logger.info("outerPackBlock  %5d  %5d", outerPackBlock.offset, outerPackBlock.size);
	logger.info("innerPackBlock  %5d  %5d", innerPackBlock.offset, innerPackBlock.size);
	logger.info("constBlock      %5d  %5d", constBlock.offset, constBlock.size);
	logger.info("seBlock         %5d  %5d", seBlock.offset, seBlock.size);
	logger.info("ctxBlock        %5d  %5d", ctxBlock.offset, ctxBlock.size);
	logger.info("mdBlock         %5d  %5d", mdBlock.offset, mdBlock.size);
	logger.info("bodyBlock       %5d  %5d", bodyBlock.offset, bodyBlock.size);
	logger.info("extBlock        %5d  %5d", extBlock.offset, extBlock.size);
	logger.info("treeBlock       %5d  %5d", treeBlock.offset, treeBlock.size);
	logger.info("litBlock        %5d  %5d", litBlock.offset, litBlock.size);
	logger.info("sLitBlock       %5d  %5d", sLitBlock.offset, sLitBlock.size);
	logger.info("epMapBlock      %5d  %5d", epMapBlock.offset, epMapBlock.size);
	logger.info("spareBlock      %5d  %5d", spareBlock.offset, spareBlock.size);
	logger.info("fgRelPgBase     %5d", fgRelPgBase);
	logger.info("fgPgCount       %5d", fgPgCount);

	logger.info("htTable         %5d", htTable.size());
}

void Symbols::dumpTable() {
    logger.info("htTable   %d", htTable.size());
    // for(const auto& e: htTable) {
    //     auto key = e.first;
    //     auto value = e.second;
    //     logger.info("%-8s  %s", std_sprintf("%s-%d", "ht", key), value.toString());
    // }
    logger.info("mdTable   %d", mdTable.size());
    for(const auto& e: mdTable) {
        auto key = e.first;
        auto value = e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", "md", key), value->toString());
    }
    logger.info("ctxTable  %d", ctxTable.size());
    for(const auto& e: ctxTable) {
        auto key = e.first;
        auto value = e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", "ctx", key), value->toString());
    }
}
void Symbols::dumpIndex() {
//    HTIndex::dump();
	MDIndex::dump();

    logger.info("HTIndex    indexSet  %d", HTIndex::indexSet.size());
    logger.info("MDIndex    indexSet  %d", MDIndex::indexSet.size());
}



template<class T>
const T& getRecord(uint16_t index, const std::map<uint16_t, T>& map) {
	if (map.contains(index)) return map.at(index);
	logger.error("Unexpeced index");
	logger.error("  index  %d", index);
	ERROR()
}


std::string Symbols::getSS(ByteBuffer& bb) {
	BlockDescriptor& block = ssBlock;
    uint16_t offset = symbolBase + block.offset * 2;
    uint16_t limit  = block.size * 2;

	uint16_t length;
	uint16_t maxLength;

    bb.position(offset);
	bb.read(length, maxLength);

	std::string ss;
    for(int i = 0; i < maxLength; i++) {
		uint8_t c = bb.get8();
		if (i < length) ss += c;
    }

	// sanity check
	if (bb.position() != (offset + limit)) {
		logger.error("Unexpected length");
		logger.error("  offset     %5d", offset);
		logger.error("  limit      %5d", limit);
		logger.error("  length     %5d", length);
		logger.error("  maxLength  %5d", maxLength);
	}
	return ss;
}
void Symbols::initializeHT(ByteBuffer& bb) {
	std::string ss = getSS(bb);

	BlockDescriptor& block = htBlock;
    uint16_t base  = symbolBase + block.offset * 2;
    uint16_t limit = base + block.size * 2;

    uint16_t lastSSIndex = 0;
    uint16_t index = 0;
    bb.position(base);
    for(;;) {
        int pos = bb.position();
        if (limit <= pos) break;

		HTRecord* record = new HTRecord;
		record->read(bb, lastSSIndex, ss);
        htTable[index] = record;

//        logger.info("ht %4d %s", index, record.toString());
        index++;
        lastSSIndex = record->ssIndex;
    }
}

uint16_t getIndex(int pos, int offset, int limit) {
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

template<class T>
static void buildTable(ByteBuffer& bb, uint32_t symbolBase, int offset, int limit_, std::map<uint16_t, T*>& table) {
    int base  = symbolBase + offset * 2;
    int limit = base + limit_ * 2;
    bb.position(base);
    for(;;) {
        int index = getIndex(bb.position() - symbolBase, offset, limit_);
        if (limit <= bb.position()) break;
		T* value = new T;
        value->read(bb);
        table[index] = value;
    }
	// sanity check
	if (bb.position() != (limit)) {
		logger.error("Unexpected length");
		logger.error("  pos        %5d", bb.position());
		logger.error("  base       %5d", base);
		logger.error("  limit      %5d", limit);
		logger.error("  index      %5d", index);
	}
}


void Symbols::initializeMD(ByteBuffer& bb) {
	BlockDescriptor& block = mdBlock;
	buildTable(bb, symbolBase, block.offset, block.size, mdTable);
}
void Symbols::initializeCTX(ByteBuffer& bb) {
	BlockDescriptor& block = ctxBlock;
	buildTable(bb, symbolBase, block.offset, block.size, ctxTable);
}
