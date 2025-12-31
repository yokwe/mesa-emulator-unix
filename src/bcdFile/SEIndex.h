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
// SEIndex.h
//

#pragma once

#include <cstdint>

#include "../util/Util.h"

#include "Index.h"

// forward declaration
struct SERecord;

// SEIndex: TYPE = Base RELATIVE ORDERED POINTER [0..3777B] TO SERecord;
// SENull: SEIndex = FIRST[SEIndex];
// typeTYPE: CSEIndex = FIRST[CSEIndex] + SIZE[nil cons SERecord];
// typeANY: CSEIndex = typeTYPE + SIZE[mode cons SERecord];

struct SEIndex : public Index<"se", SERecord> {
    static const constexpr uint16_t SE_NULL   = 0;
	static const constexpr uint16_t TYPE_TYPE = 1;
	static const constexpr uint16_t TYPE_ANY  = 2;

    SEIndex() : Index() {}
    SEIndex(uint16_t index_, const SERecord* value_) : Index(index_, value_) {}
    SEIndex(uint16_t index_) : Index(index_) {}
    
    bool isNull() const {
        return index() == SE_NULL;
    }
    bool isType() const {
        return index() == TYPE_TYPE;
    }
    bool isAny() const {
        return index() == TYPE_ANY;
    }
    std::string toString() const {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        if (isType()) return std_sprintf("%s-TYPE", prefix);
        if (isAny())  return std_sprintf("%s-ANY", prefix);
        return Index::toString();
    }
};
