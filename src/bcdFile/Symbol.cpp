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
// Symbol.cpp
//

#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Type.h"

#include "MesaByteBuffer.h"

#include "MDIndex.h"
#include "EXTIndex.h"

#include "Symbol.h"

#include "LTIndex.h"

//
// concrete definition
//
#include "HTRecord.h"

#include "BodyRecord.h"
#include "MDRecord.h"
#include "CTXRecord.h"
#include "SERecord.h"
#include "EXTRecord.h"
#include "Tree.h"


static std::string readSSTable(MesaByteBuffer& baseBB, const BlockDescriptor& block) {
    uint16_t offset = block.offset;
    uint16_t limit  = block.size;
    auto bb = baseBB.range(offset, limit);

	uint16_t length;
	uint16_t maxLength;
	bb.read(length, maxLength);

	std::string ss;
    for(int i = 0; i < maxLength; i++) {
		uint8_t c = bb.get8();
		if (i < length) ss += c;
    }

	// sanity check
	if (bb.pos() != limit) {
		logger.error("Unexpected length");
		logger.error("  pos        %5d", bb.pos());
		logger.error("  bytePos    %5d", bb.bytePos());
		logger.error("  offset     %5d", offset);
		logger.error("  limit      %5d", limit);
		logger.error("  length     %5d", length);
		logger.error("  maxLength  %5d", maxLength);
	}
	return ss;
}
static void readTable(MesaByteBuffer& baseBB, const BlockDescriptor& block, std::map<uint16_t, HTRecord*>& table, const BlockDescriptor& ssBlock) {
	const std::string ss = readSSTable(baseBB, ssBlock);

    uint16_t offset = block.offset;
    uint16_t limit  = block.size;
    auto bb = baseBB.range(offset, limit);

    uint16_t lastSSIndex = 0;
    uint16_t index = 0;
    for(;;) {
        int pos = bb.pos();
        if (limit <= pos) break;

		HTRecord* record = new HTRecord;
		record->read(bb);
		// set value of record
		record->value = ss.substr(lastSSIndex, record->ssIndex - lastSSIndex);

        table[index++] = record;
        lastSSIndex = record->ssIndex;
    }
	// sanity check
	if (bb.pos() != limit) {
		logger.error("Unexpected length");
		logger.error("  pos        %5d", bb.pos());
		logger.error("  bytePos    %5d", bb.bytePos());
		logger.error("  offset     %5d", block.offset);
		logger.error("  size       %5d", block.size);
	}
}
template<class T>
static void readTable(MesaByteBuffer& baseBB, const BlockDescriptor& block, std::map<uint16_t, T*>& table, std::string prefix) {
	auto bb = baseBB.range(block.offset, block.size);
	auto limit = block.size;
    for(;;) {
        int index = bb.pos();
        if (limit <= index) break;
		T* value = new T;
        value->read(bb);
        table[index] = value;
    }
	// sanity check
	if (bb.pos() != limit) {
		logger.error("Unexpected position");
		logger.error("  prefix     %s", prefix);
		logger.error("  pos        %5d", bb.pos());
		logger.error("  bytePos    %5d", bb.bytePos());
		logger.error("  offset     %5d", block.offset);
		logger.error("  size       %5d", block.size);
		ERROR()
	}
}

Symbol Symbol::getInstance(MesaByteBuffer bb) {
	// sanity check
	checkVersionIdent(bb);

	Symbol symbol;

	symbol.read(bb);

	// read tables
	::readTable(bb, symbol.bodyBlock, symbol.bodyTable, "body");
	::readTable(bb, symbol.ctxBlock, symbol.ctxTable, "ctx");
	::readTable(bb, symbol.extBlock, symbol.extTable, "ext");
	::readTable(bb, symbol.htBlock, symbol.htTable, symbol.ssBlock);
	::readTable(bb, symbol.litBlock, symbol.litTable, "lit");
	::readTable(bb, symbol.mdBlock, symbol.mdTable, "md");
	::readTable(bb, symbol.seBlock, symbol.seTable, "se");
	::readTable(bb, symbol.treeBlock, symbol.treeTable, "tree");

	// set index value
	BTIndex::setValue(symbol.bodyTable);
	CTXIndex::setValue(symbol.ctxTable);
	EXTIndex::setValue(symbol.extTable);
	HTIndex::setValue(symbol.htTable);
	LTIndex::setValue(symbol.litTable);
	MDIndex::setValue(symbol.mdTable);
	SEIndex::setValue(symbol.seTable);
	TreeIndex::setValue(symbol.treeTable);

	return symbol;
}

void Symbol::checkVersionIdent(MesaByteBuffer &bb) {
    auto oldPos = bb.pos();
    auto word = bb.get16();
    bb.pos(oldPos);
    if (word == Symbol::VersionID) return;
    logger.error("Unexpected version  %d", word);
    ERROR()
}

MesaByteBuffer& Symbol::read(MesaByteBuffer& bb) {
	uint16_t u10;

	bb.read(versionIdent, version, creator, sourceVersion, u10, importCtx, outerCtx);
	bb.read(hvBlock, htBlock, ssBlock, outerPackBlock, innerPackBlock, constBlock);
	bb.read(seBlock, ctxBlock, mdBlock, bodyBlock, extBlock, treeBlock, litBlock, sLitBlock, epMapBlock, spareBlock);
	bb.read(fgRelPgBase, fgPgCount);

	definitionsFile = bitField(u10, 15);

	directoryCtx.index(bitField(u10, 1, 15));

	return bb;
}

void Symbol::dump() {
    logger.info("versionIdent    %5d", versionIdent);
	logger.info("version            %s", version.toString());
	logger.info("creator            %s", creator.toString());
	logger.info("sourceVersion      %s", creator.toString());
	logger.info("definitionsFile    %s", definitionsFile ? "YES" : "NO");
	logger.info("directoryCtx       %s", directoryCtx.toString());
	logger.info("importCtx          %s", importCtx.toString());
	logger.info("outerCtx           %s", outerCtx.toString());

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

	logger.info("bodyTable       %5d", bodyTable.size());
	logger.info("ctxTable        %5d", ctxTable.size());
	logger.info("extTable        %5d", extTable.size());
	logger.info("htTable         %5d", htTable.size());
	logger.info("litTable        %5d", litTable.size());
	logger.info("mdTable         %5d", mdTable.size());
	logger.info("seTable         %5d", seTable.size());
	logger.info("treeTable       %5d", treeTable.size());
}

template <class T>
static void dumpTable(const char* prefix, const std::map<uint16_t, T*>& map) {
	for (auto const& [key, value] : map) {
        logger.info("%-8s  %s", std_sprintf("%s-%d", prefix, key), value->toString());
	}
}
void Symbol::dumpTable() {
	::dumpTable("body", bodyTable);
	::dumpTable("ctx", ctxTable);
	::dumpTable("ext", extTable);
	::dumpTable("ht", htTable);
	::dumpTable("lt", litTable);
	::dumpTable("md", mdTable);
	::dumpTable("se", seTable);
	::dumpTable("tree", treeTable);
}


static SEIndex seNull{SEIndex::SE_NULL, 0};
SEIndex Symbol::nextSei(SEIndex sei) {
	// sanity check
	if (sei.value().tag != SERecord::Tag::ID) ERROR()

	using ID = SERecord::ID;
	for(auto i = seTable.cbegin(); i != seTable.cend(); i++) {
		auto [key, value] = *i;
		if (key == sei.index()) {
			const auto& id = value->toID();
			// next for SEQUENTIAL
			auto next = i;
			next++;
			switch(id.tag) {
				case ID::Tag::LINKED:
					return id.toLINKED().link;
				case ID::Tag::SEQUENTIAL:
					return SEIndex(next->first, next->second);;
				case ID::Tag::TERMINAL:
					return seNull;
				default:
					ERROR()
			}
		}
	}
	ERROR()
}

//UnderType: PROC [h: Handle, type: SEIndex] RETURNS [CSEIndex] = {
//  sei: SEIndex � type;
//  WHILE sei # SENull DO
//    WITH se: h.seb[sei] SELECT FROM
//      id => {IF se.idType # typeTYPE THEN ERROR; sei � SymbolOps.ToSei[se.idInfo]};
//      ENDCASE => EXIT;
//    ENDLOOP;
//  RETURN [LOOPHOLE[sei, CSEIndex]]};
SEIndex Symbol::underType(SEIndex sei) {
	while (!sei.isNull()) {
		if (sei.value().isID()) {
			auto se = sei.value().toID();
			if (!se.idType.isType()) ERROR()
			sei = toSEIndex(se.idInfo);
		} else {
			break;
		}
	}
	return sei;
}

SEIndex Symbol::typeLink(SEIndex sei) {
	sei = underType(sei);
	const auto& cons = sei.value().toCONS();
	if (cons.isRECORD()) {
		const auto& record = cons.toRECORD();
		if (record.tag == SERecord::CONS::RECORD::Tag::LINKED) {
			return record.toLINKED().linkType;
		}
	}
	return seNull;
}    

//XferMode: PROC [h: Handle, type: SEIndex] RETURNS [TransferMode] = {
//  sei: CSEIndex = UnderType[h, type];
//  RETURN [WITH t: h.seb[sei] SELECT FROM transfer => t.mode, ENDCASE => none]};
TransferMode Symbol::xferMode(SEIndex type) {
	auto sei = underType(type);
	const auto& cons = sei.value().toCONS();
	return cons.tag == TypeClass::TRANSFER ? cons.toTRANSFER().mode : TransferMode::NONE;
}

SEIndex Symbol::toSEIndex(uint16_t index) {
	if (seTable.contains(index)) {
		return SEIndex(index, seTable.at(index));
	} else {
		ERROR();
//		return SEIndex(index);
	}
}

static BTIndex btNull{BTIndex::BT_NULL, 0};
BTIndex Symbol::toBTIndex(uint16_t index) {
	if (bodyTable.contains(index)) {
		return BTIndex(index, bodyTable.at(index));
	} else {
		return btNull;
	}
}

static EXTIndex extNull{EXTIndex::EXT_NULL, 0};
EXTIndex Symbol::toEXTIndex(SEIndex sei) {
	for(const auto& [key, value]: extTable) {		
		if (value->sei == sei) return EXTIndex(key, value);
	}
	return extNull;
}
