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
// Index.h
//

#pragma once

#include <set>
#include <map>

#include "../util/Util.h"

#include "../util/ByteBuffer.h"


template <StringLiteral PREFIX, class T>
struct Index : public ByteBuffer::Readable, public HasToString {
    static inline const char* prefix = PREFIX;

    static inline std::set<Index*>      indexSet;

    static void addIndex(Index* index) {
        if (indexSet.contains(index)) {
            // unexpected
            logger.error("Duplicate key");
            logger.error("  prefix  %s", prefix);
            logger.error("  index   %p  %d", index, index->index);
            ERROR()
        }
        // after index has assigned register to map
        indexSet.insert(index);
    }
    static void removeIndex(Index* reference) {
        if (indexSet.contains(reference)) {
            indexSet.erase(reference);
        }
    }
    static void clear() {
        indexSet.clear();
    }
    static void setValue(std::map<uint16_t, T>& valueMap) {
        for(auto i = indexSet.begin(); i != indexSet.end(); i++) {
            Index* p = *i;
            if (p->noIndex) continue;
            auto index = p->index;
            if (valueMap.contains(index)) {
                p->value   = &valueMap[index];
            }
        }
    }
    static void dump() {
        for(const auto& e: indexSet) {
            if (e->noIndex) continue;
            logger.info("dump  %-8s  %s", e->toStringIndex(), e->toString());
        }
    }

protected:
    uint16_t index;
    T*       value;
    bool     noIndex;

public:
    // default constructor
    Index() : ByteBuffer::Readable(), index(55555), value(0), noIndex(true) {
        addIndex(this);
    }
    // copy constructor
    Index(const Index& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
       addIndex(this);
    }
    // move constructor
    Index(Index&& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
        addIndex(this);
    }
    ~Index() {
        removeIndex(this);
    }

    Index& operator=(const Index& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
        return *this;
    }

    void setIndex(uint16_t index_) {
        if (noIndex) {
            index   = index_;
            noIndex = false;
        } else {
            logger.error("index already has value");
            logger.error("  %s-%d", prefix, index);
            logger.error("  index_  %d", index_);
            ERROR()
        }
    }
    uint16_t getIndex() const {
        if (noIndex) {
            logger.error("index has no value");
            ERROR()
        } else {
            return index;
        }
    }

    const T& getValue() const {
        if (value) return *value;
        logger.error("index has no value");
        logger.error("  %s-%d", prefix, index);
        ERROR()
    }
    const T& operator*() const noexcept {
        if (value) return *value;
        logger.error("index has no value");
        logger.error("  %s-%d", prefix, index);
        ERROR()
    }
    operator uint16_t() const {
        return index;
    }
    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(index);
        noIndex = false;
        return bb;
    }
    std::string toString() const override {
        if (value) {
            constexpr auto is_string  = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;
            if constexpr(is_string) {
                return *value;
            } else {
                return value->toString();
            }
        }
        return std_sprintf("%s-%d", prefix, index);
    }
    std::string toStringIndex() const {
        return std_sprintf("%s-%d", prefix, index);
    }
};
