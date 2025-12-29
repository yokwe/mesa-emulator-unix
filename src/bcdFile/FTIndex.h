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
// FTIndex.h
//

#pragma once

#include <cstdint>

#include "../util/Util.h"

#include "Index.h"

// forward declaration
struct FTRecord;

// FTIndex: TYPE = Table.Base RELATIVE POINTER [0..tLimit] TO FTRecord;
// FTNull: FTIndex = LAST[FTIndex];
// FTSelf: FTIndex = LAST[FTIndex] - 1;
struct FTIndex : public Index<"ft", FTRecord> {
    static const constexpr uint16_t FT_NULL = T_LIMIT;
    static const constexpr uint16_t FT_SELF = T_LIMIT - 1;

    bool isNull() const {
        return index() == FT_NULL;
    }
    bool isSelf() const {
        return index() == FT_SELF;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        if (isSelf()) return std_sprintf("%s-SELF", prefix);
        return Index::toString();
    }
};
