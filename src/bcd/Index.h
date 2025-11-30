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

constexpr const uint16_t T_LIMIT = 0177777;

template <StringLiteral PREFIX, class T>
struct Index : public ByteBuffer::Readable, public HasToString {
    static inline const char* prefix = PREFIX;

    static inline std::set<Index*> indexSet;

    static void addIndex(Index* index) {
        if (indexSet.contains(index)) {
            // unexpected
            logger.error("Duplicate key");
            logger.error("  index   %s  %s", index->toString(), index);
            ERROR()
        }
        // after index has assigned register to map
        indexSet.insert(index);
    }
    static void removeIndex(Index* index) {
        if (indexSet.contains(index)) {
            indexSet.erase(index);
        // } else {
        //     logger.error("Unexpected index");
        //     logger.error("  index   %s  %p", index->toString(), index->_value);
        //     ERROR()
        }
    }
    static void clear() {
        indexSet.clear();
    }
    static void setValue(const std::map<uint16_t, T*>& valueMap) {
        for(auto i = indexSet.begin(); i != indexSet.end(); i++) {
            Index* p = *i;
            if (p->noIndex) continue;
            auto index = p->_index;
            if (valueMap.contains(index)) {
                p->_value   = valueMap.at(index);
            }
        }
    }
    static void dump() {
        for(const auto& e: indexSet) {
            if (e->noIndex) continue;
            logger.info("dump  %s", e->Index::toString());
        }
    }
    static void stats() {
        int countHasIndex = 0;
        int countHasValue = 0;
        for(const auto& e: indexSet) {
            if (e->noIndex) continue;
            countHasIndex++;
            if (e->_value) countHasValue++;
        }

        logger.info("index  %-3s  %5d / %5d / %5d", prefix, countHasValue, countHasIndex, indexSet.size());
    }

private:
    uint16_t _index;
    const T* _value;
    bool     noIndex;

public:
    // default constructor
    Index() : _index(55555), _value(0), noIndex(true) {
        addIndex(this);
    }
    Index(uint16_t index_, const T* value_) : _index(index_), _value(value_), noIndex(value_ == 0) {}
    Index(uint16_t index_) : Index(index_, 0) {}

    // copy constructor
    Index(const Index& that) {
        this->_index  = that._index;
        this->_value  = that._value;
        this->noIndex = that.noIndex;
       addIndex(this);
    }
    // move constructor
    Index(Index&& that) {
        this->_index  = that._index;
        this->_value  = that._value;
        this->noIndex = that.noIndex;
        addIndex(this);
    }
    ~Index() {
        removeIndex(this);
    }

    //
    // operator
    //
    Index& operator=(const Index& that) {
        this->_index  = that._index;
        this->_value  = that._value;
        this->noIndex = that.noIndex;
        return *this;
    }
    bool operator==(const Index& that) const {
        return this->_index == that._index && this->_value == that._value;
    }

    //
    // index
    //
    uint16_t index() const {
        if (noIndex) {
            logger.error("index has no value");
            ERROR()
        } else {
            return _index;
        }
    }
    void index(uint16_t index) {
        if (noIndex) {
            _index   = index;
            noIndex = false;
        } else {
            logger.error("index already has value");
            logger.error("  %s", toString());
            logger.error("  index  %d", index);
            ERROR()
        }
    }

    //
    // value
    //
    bool hasValue() const {
        return _value;
    }
    const T& value() const {
        if (hasValue()) {
            return *_value;
        } else {
            logger.error("value has no value");
            logger.error("  %s", toString());
            logger.error("  index  %d", _index);
            ERROR()
        }
    }

    ByteBuffer& read(ByteBuffer& bb) override {
        uint16_t newValue;
        bb.read(newValue);
        index(newValue);
        return bb;
    }
    std::string toString() const override {
        std::string mark = (hasValue()) ? "-" : "?";
        return std_sprintf("%s%s%d", prefix, mark, _index);
    }
};
