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
// MDRecord.h
//

#pragma once

#include <cstdint>
#include <string>

#include "BCD.h"
#include "Index.h"
#include "Timestamp.h"
#include "HTRecord.h"

// forward declaration
struct MDRecord;

//MDIndex: TYPE = Base RELATIVE ORDERED POINTER [0..Limit) TO MDRecord;
//MDNull: MDIndex = LAST[MDIndex];
//OwnMdi: MDIndex = FIRST[MDIndex];
struct MDIndex : public Index<"md", MDRecord> {
    static const constexpr uint16_t MD_NULL = T_LIMIT;
    
    bool isNull() const {
        return index == MD_NULL;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        return Index::toString();
    }
};

//MDRecord: TYPE = RECORD [
//  stamp: TimeStamp.Stamp,
//  moduleId: HTIndex,		-- hash entry for module name
//  fileId: HTIndex,		-- hash entry for file name
//  shared: BOOLEAN,		-- overrides PRIVATE, etc.
//  exported: BOOLEAN,
//  ctx: IncludedCTXIndex,	-- context of copied entries
//  defaultImport: CTXIndex,	-- unnamed imported instance
//  file: FileIndex];		-- associated file
struct MDRecord : public HasToString {
    using CTXIndex = uint16_t; // FIXME

    Timestamp  stamp;
    HTIndex    moduleId;
    HTIndex    fileId;
    bool       shared;
    bool       exported;
    CTXIndex   ctx;
    CTXIndex   defaultImport;
    uint16_t   fileIndex;  // this is not FTIndex but index of ftTable element.  0 means first entry of ftTable. 1 means second entry of ftTable

    void read(ByteBuffer& bb) {
        uint16_t word;
        bb.read(stamp, moduleId, word, ctx, defaultImport, fileIndex);

        fileId.setIndex(bitField(word, 0, 12));
        shared   = bitField(word, 13);
        exported = bitField(word, 14, 15);
    }

    std::string toString() const override {
        auto moduleId_ = moduleId.toValue();
        auto fileId_ = fileId.getValue();
    
        return std_sprintf("[[%s]  %s  %s  %s%s  %5d  %5d  %5d]",
            stamp.toString(), moduleId.toValue(), fileId.toValue(),
            shared ? "S" : "", exported ? "E" : "", ctx, defaultImport, fileIndex);
    }
};