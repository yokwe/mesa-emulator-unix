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

#include "SymbolsIndex.h"


//
// Common type used in Symbols
//

//   ContextLevel: TYPE = [0..7];
//     lZ: ContextLevel = 0;	-- context level of non-frame records
//     lG: ContextLevel = 1;	-- context level of global frame
//     lL: ContextLevel = lG+1;	-- context level of outer procedures
enum class ContextLevel : uint16_t {
	ENUM_VALUE(ContextLevel, LZ)
	ENUM_VALUE(ContextLevel, LG)
	ENUM_VALUE(ContextLevel, LL)
};
std::string toString(ContextLevel valeu);

//  ExtensionType: TYPE = {value, form, default, none};
enum class ExtensionType {
	ENUM_VALUE(ExtensionType, VALUE)
	ENUM_VALUE(ExtensionType, FORM)
	ENUM_VALUE(ExtensionType, DEFAULT)
	ENUM_VALUE(ExtensionType, NONE)
};
std::string toString(ExtensionType value);

enum class TypeClass : uint16_t {
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
	//
	ENUM_VALUE(TypeClass, FIXEDSEQUENCE)
};
std::string toString(TypeClass value);

// TransferMode: TYPE = {proc, port, signal, error, process, program, none};
enum class TransferMode : uint16_t {
	ENUM_VALUE(TransferMode, PROC)
	ENUM_VALUE(TransferMode, PORT)
	ENUM_VALUE(TransferMode, SIGNAL)
	ENUM_VALUE(TransferMode, ERROR_)
	ENUM_VALUE(TransferMode, PROCESS)
	ENUM_VALUE(TransferMode, PROGRAM)
	ENUM_VALUE(TransferMode, NONE)
};
std::string toString(TransferMode value);



struct BTRecord;
struct CTXRecord;
struct EXTRecord;
struct HTRecord;
struct LTRecord;
struct MDRecord;
struct SERecord;
struct TreeNode;

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
	ByteBuffer& read(ByteBuffer& bb);

	std::string getSS(ByteBuffer& bb);

	void initializeBT(ByteBuffer& bb);
	void initializeCTX(ByteBuffer& bb);
	void initializeEXT(ByteBuffer& bb);
	void initializeHT(ByteBuffer& bb);
	void initializeLT(ByteBuffer& bb);
	void initializeMD(ByteBuffer& bb);
	void initializeSE(ByteBuffer& bb);
	void initializeTree(ByteBuffer& bb);

public:
	// VersionID: CARDINAL = 08140; -- AMesa/14.0/Compiler/Friends/SymbolSegment.mesa
	static const uint16_t VersionID = 8140;

    //  altoBias: CARDINAL = 1;  -- AMesa/14.0/Compiler/Friends/FilePack.mesa
	static const uint16_t ALTO_BIAS = 1;

	static Symbols getInstance(ByteBuffer& bb, int offset);

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

    // contents of above table
	std::map<uint16_t, BTRecord*>   btTable;
	std::map<uint16_t, CTXRecord*>  ctxTable;
	std::map<uint16_t, EXTRecord*>  extTable;
	std::map<uint16_t, HTRecord*>   htTable;
	std::map<uint16_t, LTRecord*>   ltTable;
	std::map<uint16_t, MDRecord*>   mdTable;
	std::map<uint16_t, SERecord*>   seTable;
	std::map<uint16_t, TreeNode*>   treeTable;


	void dump();
    void dumpTable();
    void dumpIndex();

	SEIndex  toSEIndex(uint16_t index);
	BTIndex  toBTIndex(uint16_t index);
	EXTIndex toEXTIndex(SEIndex sei);

	SEIndex nextSei(SEIndex sei);
	SEIndex sequential(SEIndex sei);

	TransferMode xferMode(SEIndex sei);
	SEIndex underType(SEIndex sei);
};
