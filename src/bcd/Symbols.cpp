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

//#include "BTRecord.h"
#include "CTXRecord.h"
#include "EXTRecord.h"
#include "HTRecord.h"
#include "LTRecord.h"
#include "MDRecord.h"
#include "SERecord.h"
#include "EXTRecord.h"
//#include "Tree.h"

#include "Symbols.h"


Symbols Symbols::getInstance(ByteBuffer &bb, int offset) {
	HTIndex::clear();

    Symbols symbols;

    bb.position(offset);
    symbols.read(bb);

//	symbols.initializeBT(bb);
	symbols.initializeCTX(bb);
	symbols.initializeEXT(bb);
	symbols.initializeHT(bb);
	symbols.initializeLT(bb);
	symbols.initializeMD(bb);
	symbols.initializeSE(bb);
//	symbols.initializeTree(bb);


//	BTIndex::setValue(symbols.btTable);
	CTXIndex::setValue(symbols.ctxTable);
	EXTIndex::setValue(symbols.extTable);
	HTIndex::setValue(symbols.htTable);
	LTIndex::setValue(symbols.ltTable);
	MDIndex::setValue(symbols.mdTable);
	SEIndex::setValue(symbols.seTable);
//	TreeIndex::setValue(symbols.treeTable);

//	BTIndex::stats();
	CTXIndex::stats();
	EXTIndex::stats();
	HTIndex::stats();
	LTIndex::stats();
	MDIndex::stats();
	SEIndex::stats();
//	TreeLInk::stats();

    return symbols;
}

ByteBuffer& Symbols::read(ByteBuffer& bb) {
	symbolBase = bb.position();

	uint16_t u10;

	bb.read(versionIdent, version, creator, sourceVersion, u10, importCtx, outerCtx);
	bb.read(hvBlock, htBlock, ssBlock, outerPackBlock, innerPackBlock, constBlock);
	bb.read(seBlock, ctxBlock, mdBlock, bodyBlock, extBlock, treeBlock, litBlock, sLitBlock, epMapBlock, spareBlock);
	bb.read(fgRelPgBase, fgPgCount);

	definitionsFile = bitField(u10, 15);

	directoryCtx.index(bitField(u10, 1, 15));

	return bb;
}

void Symbols::dump() {
    logger.info("symbolBase      %5d  %5d", symbolBase, symbolBase / Environment::bytesPerPage);
    logger.info("versionIdent    %5d", versionIdent);
	logger.info("version            %s", version.toString());
	logger.info("creator            %s", creator.toString());
	logger.info("sourceVersion      %s", creator.toString());
	logger.info("definitionsFile    %s", definitionsFile ? "YES" : "NO");
	logger.info("directoryCtx       %s", directoryCtx.Index::toString());
	logger.info("importCtx          %s", importCtx.Index::toString());
	logger.info("outerCtx           %s", outerCtx.Index::toString());

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

	logger.info("btTable         %5d", btTable.size());
	logger.info("ctxTable        %5d", ctxTable.size());
	logger.info("extTable        %5d", extTable.size());
	logger.info("htTable         %5d", htTable.size());
	logger.info("ltTable         %5d", ltTable.size());
	logger.info("mdTable         %5d", mdTable.size());
	logger.info("seTable         %5d", seTable.size());
	logger.info("treeTable       %5d", treeTable.size());
}

template <class T>
void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
    for(const auto& e: map) {
        auto key = e.first;
        auto value = e.second;
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
    }
}
void Symbols::dumpTable() {
//	::dumpTable("bt", btTable);
//	::dumpTable("ctx", ctxTable);
//	::dumpTable("ext", extTable);
//	::dumpTable("ht", htTable);
//	::dumpTable("lt", ltTable);
//	::dumpTable("md", mdTable);
//	::dumpTable("se", seTable);
//	::dumpTable("tree", treeTable);
}
void Symbols::dumpIndex() {
//	BTIndex::dump();
//	CTXIndex::dump();
//	HTIndex::dump();
//	EXTIndex::dump();
//	LTIndex::dump();
//	MDIndex::dump();
//	SEIndex::dump();
//	TreeIndex::dump();
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
static void buildTable(ByteBuffer& bb, uint32_t symbolBase, int offset, int limit_, std::map<uint16_t, T*>& table, std::string prefix) {
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
		logger.error("  prefix     %s", prefix);
		logger.error("  pos        %5d", bb.position());
		logger.error("  base       %5d", base);
		logger.error("  limit      %5d", limit);
		logger.error("  index      %5d", index);
	}
}


void Symbols::initializeLT(ByteBuffer& bb) {
	BlockDescriptor& block = litBlock;
	buildTable(bb, symbolBase, block.offset, block.size, ltTable, "lt");
}
void Symbols::initializeMD(ByteBuffer& bb) {
	BlockDescriptor& block = mdBlock;
	buildTable(bb, symbolBase, block.offset, block.size, mdTable, "md");
}
void Symbols::initializeCTX(ByteBuffer& bb) {
	BlockDescriptor& block = ctxBlock;
	buildTable(bb, symbolBase, block.offset, block.size, ctxTable, "ctx");
}
void Symbols::initializeSE(ByteBuffer& bb) {
	BlockDescriptor& block = seBlock;
	buildTable(bb, symbolBase, block.offset, block.size, seTable, "se");
}
void Symbols::initializeEXT(ByteBuffer& bb) {
	BlockDescriptor& block = extBlock;
	buildTable(bb, symbolBase, block.offset, block.size, extTable, "ext");
}


#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

std::string toString(ContextLevel value) {
    static std::map<ContextLevel, std::string> map {
        ENUM_VALUE(ContextLevel, LZ)
        ENUM_VALUE(ContextLevel, LG)
        ENUM_VALUE(ContextLevel, LL)
    };

    if (map.contains(value)) return map[value];
    return std_sprintf("%2d", (uint16_t)value);
    // logger.error("Unexpected value");
    // logger.error("  value  %d", (uint16_t)value);
    // ERROR();
}

std::string toString(ExtensionType value) {
    static std::map<ExtensionType, std::string> map {
		ENUM_VALUE(ExtensionType, VALUE)
		ENUM_VALUE(ExtensionType, FORM)
		ENUM_VALUE(ExtensionType, DEFAULT)
		ENUM_VALUE(ExtensionType, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string toString(TypeClass value) {
    static std::map<TypeClass, std::string> map {
        ENUM_VALUE(TypeClass, MODE)
        ENUM_VALUE(TypeClass, BASIC)
        ENUM_VALUE(TypeClass, ENUMERATED)
        ENUM_VALUE(TypeClass, RECORD)
        ENUM_VALUE(TypeClass, REF)
        //
        ENUM_VALUE(TypeClass, ARRAY)
        ENUM_VALUE(TypeClass, ARRAYDESC)
        ENUM_VALUE(TypeClass, TRANSFER)
        ENUM_VALUE(TypeClass, DEFINITION)
        ENUM_VALUE(TypeClass, UNION)
        //
        ENUM_VALUE(TypeClass, SEQUENCE)
        ENUM_VALUE(TypeClass, RELATIVE)
        ENUM_VALUE(TypeClass, SUBRANGE)
        ENUM_VALUE(TypeClass, LONG) 
        ENUM_VALUE(TypeClass, REAL)
        //
        ENUM_VALUE(TypeClass, OPAQUE)
        ENUM_VALUE(TypeClass, ZONE)
        ENUM_VALUE(TypeClass, ANY)
        ENUM_VALUE(TypeClass, NIL)
        ENUM_VALUE(TypeClass, BITS)
        ENUM_VALUE(TypeClass, BITS)
		//
		ENUM_VALUE(TypeClass, FIXEDSEQUENCE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
