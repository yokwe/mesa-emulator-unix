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
// Perf.cpp
//

#include <vector>

#include "Util.h"
static const Logger logger(__FILE__);

#include "Perf.h"

namespace perf {

#include "Perf.inc"

// output aligned name and value
// value can be very large number. output with thousands separator
void dump() {
    std::vector<std::pair<std::string, std::string>> outputs;
    for(const auto& e: all) {
        auto value = formatWithCommas(e.value);
        outputs.push_back(std::make_pair(e.name, value));
    }

    size_t firstLen  = 0;
    size_t secondLen = 0;
    for(auto& e: outputs) {
        firstLen  = std::max(firstLen, e.first.length());
        secondLen = std::max(secondLen, e.second.length());
    }
    std::string format = std_sprintf("%%-%ds = %%%ds", firstLen, secondLen);
    for(auto& e: outputs) {
        logger.info(format.c_str(), e.first, e.second);
    }
}
void dump(const std::string& group) {
    std::vector<std::pair<std::string, std::string>> outputs;
    for(const auto& e: all) {
        if (e.group != group) continue;
        auto value = formatWithCommas(e.value);
        outputs.push_back(std::make_pair(e.name, value));
    }

    size_t firstLen  = 0;
    size_t secondLen = 0;
    for(auto& e: outputs) {
        firstLen  = std::max(firstLen, e.first.length());
        secondLen = std::max(secondLen, e.second.length());
    }
    std::string format = std_sprintf("%%-%ds = %%%ds", firstLen, secondLen);
    for(auto& e: outputs) {
        logger.info(format.c_str(), e.first, e.second);
    }
}

void clear() {
    for(const auto& e: all) {
        e.value = 0;
    }
}

}