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
// MDIndex.h
//

#pragma once

#include <cstdint>

#include "../util/Util.h"

#include "Index.h"

// forward declaration
struct MDRecord;

// MDIndex: TYPE = Base RELATIVE ORDERED POINTER [0..Limit) TO MDRecord;
// MDNull: MDIndex = LAST[MDIndex];
// OwnMdi: MDIndex = FIRST[MDIndex];
struct MDIndex : public Index<"md", MDRecord> {
    static const constexpr uint16_t MD_NULL = T_LIMIT;
    static const constexpr uint16_t MD_OWN  = 0;
    
    bool isNull() const {
        return index() == MD_NULL;
    }
    bool isOwn() const {
        return index() == MD_OWN;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        if (isOwn())  return std_sprintf("%s-OWN", prefix);
        return Index::toString();
    }
};
