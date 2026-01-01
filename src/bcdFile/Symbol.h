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
// Symbol.h
//

#pragma once

#include <cstdint>

#include "../util/Util.h"

#include "Type.h"
#include "LTRecord.h"
#include "MesaByteBuffer.h"
#include "CTXIndex.h"
#include "SEIndex.h"

//
// forward declaration
//
struct BodyRecord;
struct HTRecord;
struct MDRecord;
struct CTXRecord;
struct SERecord;
struct EXTRecord;
struct LTRecord;
struct TreeNode;

//   WordOffset: TYPE = CARDINAL;
//   BlockDescriptor: TYPE = RECORD [offset: WordOffset, size: CARDINAL];
struct BlockDescriptor : public MesaByteBuffer::HasRead, public HasToString {
	uint16_t offset;
	uint16_t size;

	BlockDescriptor(): offset(0), size(0) {}

	MesaByteBuffer& read(MesaByteBuffer& bb) override {
		bb.read(offset, size);
		return bb;
	}
	std::string toString() const override {
		return std_sprintf("[%5d %5d]", offset, size);
	}
};

class Symbol : public MesaByteBuffer::HasRead {
    MesaByteBuffer& read(MesaByteBuffer& bb) override;

public:
    // VersionID: CARDINAL = 08140; -- AMesa/14.0/Compiler/Friends/SymbolSegment.mesa
    static const uint16_t VersionID = 8140;

    //  altoBias: CARDINAL = 1;  -- AMesa/14.0/Compiler/Friends/FilePack.mesa
    static const uint16_t ALTO_BIAS = 1;

	//-- codes identifying the basic types (extensible)
	//codeANY: CARDINAL = 0;
	//codeINT: CARDINAL = 1;
	//codeCHAR: CARDINAL = 2;
	static const uint16_t CODE_ANY  = 0;
	static const uint16_t CODE_INT  = 1;
	static const uint16_t CODE_CHAR = 2;

	static Symbol getInstance(MesaByteBuffer bb);
	static void checkVersionIdent(MesaByteBuffer& bb);

	uint16_t        versionIdent;    // version of symbol table structure
	Timestamp       version;         // identifies object file
	Timestamp       creator;         // identifies compiler
	Timestamp       sourceVersion;   // identifies source file
	bool            definitionsFile;
	CTXIndex        directoryCtx;
	CTXIndex        importCtx;
	CTXIndex        outerCtx;
	BlockDescriptor hvBlock;         // hash vector
	BlockDescriptor htBlock;         // hash table
	BlockDescriptor ssBlock;         // packed string of all identifiers
	BlockDescriptor outerPackBlock;  // outer pack
	BlockDescriptor innerPackBlock;  // inner pack
	BlockDescriptor constBlock;      // constant table
	BlockDescriptor seBlock;         // semantic entry table
	BlockDescriptor ctxBlock;        // context table
	BlockDescriptor mdBlock;         // module table
	BlockDescriptor bodyBlock;       // body table
	BlockDescriptor extBlock;        // extension table
	BlockDescriptor treeBlock;       // tree table
	BlockDescriptor litBlock;        // literal table
	BlockDescriptor sLitBlock;       // string literal table
	BlockDescriptor epMapBlock;      // unused
	BlockDescriptor spareBlock;      // unused
	uint16_t        fgRelPgBase;     // starting page of fine grain table
	uint16_t        fgPgCount;       // length of fine grain table

	// contents of above table
	std::map<uint16_t, BodyRecord*> bodyTable;
	std::map<uint16_t, CTXRecord*>  ctxTable;
	std::map<uint16_t, EXTRecord*>  extTable;
	std::map<uint16_t, HTRecord*>   htTable;
	std::map<uint16_t, LTRecord*>   litTable;
	std::map<uint16_t, MDRecord*>   mdTable;
	std::map<uint16_t, SERecord*>   seTable;
	std::map<uint16_t, TreeNode*>   treeTable;


    void dump();
    void dumpTable();
    void dumpIndex();

	// utility methods
	SEIndex nextSei(SEIndex sei);
	SEIndex underType(SEIndex sei);
	SEIndex toSEIndex(uint16_t index);
};
