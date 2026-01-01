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
// BCD.h
//

#pragma once

#include <cstdint>
#include <string>

#include "MesaByteBuffer.h"

#include "Type.h"
#include "FTIndex.h"
#include "FTRecord.h"
#include "SGRecord.h"
#include "ENRecord.h"
#include "MTRecord.h"

class BCD : public MesaByteBuffer::HasRead {
public:
    static const constexpr uint16_t VersionID = 6103;

    static BCD getInstance(MesaByteBuffer& bb);
    static void checkVersionIdent(MesaByteBuffer& bb);

	uint16_t  versionIdent;
	Timestamp version;
	Timestamp creator;

    FTIndex   sourceFile;
    FTIndex   unpackagedFile;

    uint16_t  nConfigs;
	uint16_t  nModules;
	uint16_t  nImports;
	uint16_t  nExports;
	uint16_t  nPages;
	bool      definitions;
	bool      repackaged;
	bool      typeExported;
	bool      tableCompiled;
	uint16_t  spare4;
	uint16_t  firstDummy;
	uint16_t  nDummies;

    #undef BCD_TABLE
    #define BCD_TABLE(prefix) uint16_t prefix##Offset; uint16_t prefix##Limit;

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

    // contents of above table
    std::map<uint16_t, std::string*> ssTable;
    std::map<uint16_t, FTRecord*>    ftTable;
    std::map<uint16_t, SGRecord*>    sgTable;
    std::map<uint16_t, ENRecord*>    enTable;
    std::map<uint16_t, MTRecord*>    mtTable;

    void dump();
    void dumpTable();
    void dumpIndex();

    struct Range {
        const uint16_t offset;
        const uint16_t size;

        Range(uint16_t offset_, uint16_t size_) : offset(offset_), size(size_) {}
    };
    const std::vector<Range>& symbolRange() {
        return mySymbolRange;
    }
    bool hasSymbol() {
        return !mySymbolRange.empty();
    }

private:
    MesaByteBuffer& read(MesaByteBuffer& bb) override;

    std::vector<Range> mySymbolRange;
};