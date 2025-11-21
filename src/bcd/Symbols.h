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
// Symbols.h
//

#pragma once

#include <cstdint>

#include "Timestamp.h"

#include "../util/ByteBuffer.h"

#include "HTRecord.h"

//   WordOffset: TYPE = CARDINAL;
//   BlockDescriptor: TYPE = RECORD [offset: WordOffset, size: CARDINAL];
struct BlockDescriptor : public ByteBuffer::Readable, public HasToString {
	uint16_t offset;
	uint16_t size;

	BlockDescriptor(): offset(0), size(0) {}

	ByteBuffer& read(ByteBuffer& bb) override {
		bb.read(offset, size);
		return bb;
	}
	std::string toString() const override {
		return std_sprintf("[%5d %5d]", offset, size);
	}
};
// AMesa/14.0/Compiler/Friends/SymbolSegment.mesa
//   STHeader: TYPE = RECORD [
//     versionIdent: CARDINAL,
//     version: TimeStamp.Stamp,
//     creator: TimeStamp.Stamp,
//     sourceVersion: TimeStamp.Stamp,
//     definitionsFile: BOOLEAN,
//     directoryCtx, importCtx, outerCtx: Symbols.CTXIndex,
//     hvBlock: BlockDescriptor,
//     htBlock: BlockDescriptor,
//     ssBlock: BlockDescriptor,
//     outerPackBlock: BlockDescriptor,
//     innerPackBlock: BlockDescriptor,
//     constBlock: BlockDescriptor,
//     seBlock: BlockDescriptor,
//     ctxBlock: BlockDescriptor,
//     mdBlock: BlockDescriptor,
//     bodyBlock: BlockDescriptor,
//     extBlock: BlockDescriptor,
//     treeBlock: BlockDescriptor,
//     litBlock: BlockDescriptor,
//     sLitBlock: BlockDescriptor,
//     epMapBlock: BlockDescriptor,
//     spareBlock: BlockDescriptor,
//     fgRelPgBase: CARDINAL,
//     fgPgCount: [0..256]];
class Symbols : public ByteBuffer::Readable {
	std::map<uint16_t, HTRecord>   ht;

	std::string getSS(ByteBuffer& bb);
	void initializeHT(ByteBuffer& bb);
public:
	// VersionID: CARDINAL = 08140; -- AMesa/14.0/Compiler/Friends/SymbolSegment.mesa
	static const uint16_t VersionID = 8140;

    //  altoBias: CARDINAL = 1;  -- AMesa/14.0/Compiler/Friends/FilePack.mesa
	static const uint16_t ALTO_BIAS = 1;

	static Symbols getInstance(ByteBuffer& bb, int offset);

	using CTXIndex = uint16_t;

	uint32_t        symbolBase;

	uint16_t        versionIdent;
	Timestamp       version;
	Timestamp       creator;
	Timestamp       sourceVersion;
	bool            definitionsFile;
	CTXIndex        directoryCtx;
	CTXIndex        importCtx;
	CTXIndex        outerCtx;
	BlockDescriptor hvBlock;
	BlockDescriptor htBlock;
	BlockDescriptor ssBlock;
	BlockDescriptor outerPackBlock;
	BlockDescriptor innerPackBlock;
	BlockDescriptor constBlock;
	BlockDescriptor seBlock;
	BlockDescriptor ctxBlock;
	BlockDescriptor mdBlock;
	BlockDescriptor bodyBlock;
	BlockDescriptor extBlock;
	BlockDescriptor treeBlock;
	BlockDescriptor litBlock;
	BlockDescriptor sLitBlock;
	BlockDescriptor epMapBlock;
	BlockDescriptor spareBlock;
	uint16_t        fgRelPgBase;
	uint16_t        fgPgCount;

	ByteBuffer& read(ByteBuffer& bb);
	void dump();

	HTRecord getHTRecord(uint16_t index);
};
