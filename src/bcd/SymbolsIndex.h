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
// SymbolsIndex.h
//

#pragma once

#include <cstdint>
#include <string>

#include "../util/Util.h"

#include "Index.h"

//
// BTIndex
//
struct BTRecord;
//BTIndex: TYPE = Base RELATIVE POINTER [0..Limit) TO BodyRecord;
//BTNull: BTIndex = LAST[BTIndex];
struct BTIndex : public Index<"bt", BTRecord> {
//    static const constexpr uint16_t BT_NULL = T_LIMIT;
    static const constexpr uint16_t BT_NULL = 0x3FFF;
    
    BTIndex() : Index() {}
    BTIndex(uint16_t index_, const BTRecord* value_) : Index(index_, value_) {}
    BTIndex(uint16_t index_) : Index(index_) {}

    bool isNull() const {
        return index() == BT_NULL;
    }
    std::string toString() const override;
};

//
// CTXIndex
//
struct CTXRecord;
//CTXIndex: TYPE = Base RELATIVE ORDERED POINTER [0..3777B] TO CTXRecord;
//CTXNull: CTXIndex = FIRST[CTXIndex];
struct CTXIndex : public Index<"ctx", CTXRecord> {
    static const constexpr uint16_t CTX_NULL = 0;
    
    bool isNull() const {
        return index() == CTX_NULL;
    }
    std::string toString() const override;
};

//
// ExtIndex
//
struct EXTRecord;
//ExtIndex: TYPE = Base RELATIVE POINTER [0..Limit) TO ExtRecord;
//ExtNull: ExtIndex = LAST[ExtIndex];
struct EXTIndex : public Index<"ext", EXTRecord> {
    static const constexpr uint16_t EXT_NULL = T_LIMIT - 1;
    
    EXTIndex(uint16_t index_, const EXTRecord* value_) : Index(index_, value_) {}

    bool isNull() const {
        return index() == EXT_NULL;
    }
    std::string toString() const override;
};

//
// HTIndex
//
struct HTRecord; 
//HTIndex: TYPE = CARDINAL [0..Limit/2);
//HTNull: HTIndex = FIRST[HTIndex];
struct HTIndex : public Index<"ht", HTRecord> {
    static const constexpr uint16_t HT_NULL = 0;
    
    bool isNull() const {
        return index() == HT_NULL;
    }
    std::string toString() const override;
    std::string toValue() const;
};

//
// LTIndex
//
struct LTRecord;
//LTIndex: TYPE = Base RELATIVE POINTER [0..Limit) TO LTRecord;
//LTNull: LTIndex = LAST[LTIndex];
struct LTIndex : public Index<"lt", LTRecord> {
    static const constexpr uint16_t LT_NULL = T_LIMIT;
    
    bool isNull() const {
        return index() == LT_NULL;
    }
    std::string toString() const override;
};


//
// MDIndex
//
struct MDRecord;
//MDIndex: TYPE = Base RELATIVE ORDERED POINTER [0..Limit) TO MDRecord;
//MDNull: MDIndex = LAST[MDIndex];
//OwnMdi: MDIndex = FIRST[MDIndex];
struct MDIndex : public Index<"md", MDRecord> {
    static const constexpr uint16_t MD_NULL = T_LIMIT;
    static const constexpr uint16_t MD_OWN  = 0;
    
    bool isNull() const {
        return index() == MD_NULL;
    }
    std::string toString() const override;
};

//
// SEIndex
//
struct SERecord;
//SEIndex: TYPE = Base RELATIVE ORDERED POINTER [0..3777B] TO SERecord;
//SENull: SEIndex = FIRST[SEIndex];
//typeTYPE: CSEIndex = FIRST[CSEIndex] + SIZE[nil cons SERecord];
//typeANY: CSEIndex = typeTYPE + SIZE[mode cons SERecord];

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
    std::string toString() const override;
};

//
// TreeIndex
//
struct TreeNode;
//Index: TYPE = Base RELATIVE POINTER [0..Limit) TO Tree.Node;
//NullIndex: Tree.Index = FIRST[Tree.Index];
struct TreeIndex : public Index<"tr", TreeNode> {
    static const constexpr uint16_t TreeIndex_NULL = 0;
    
    bool isNull() const {
        return index() == TreeIndex_NULL;
    }
    std::string toString() const override;
};