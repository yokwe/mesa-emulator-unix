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
// trace.cpp
//

#include <iterator>
#include <vector>
#include <algorithm>
#include <cstring>

#include "Util.h"
static const Logger logger(__FILE__);

#include "ThreadControl.h"

#include "trace.h"

namespace trace {

#include "trace.inc"

static const char* toSimplePath(const char* path) {
    auto pos = strstr(path, "/src");
    return (pos == NULL) ? path : pos + 1;
}
static std::string toStringLocalTime(const std::chrono::steady_clock::time_point time_steady) {
    auto time = to_system_clock(time_steady);
    time_t temp = std::chrono::system_clock::to_time_t(time);
	auto microsecond = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1'000'000;

    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%d-%02d-%02d %02d:%02d:%02d.%06d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, microsecond);
}
std::string Event::toString(int length_group, int length_name) const {
    auto timeString = toStringLocalTime(time);
    auto nameString = std_sprintf("%s  %s", group, name);
    auto locationString = std_sprintf("%5d  %s", line, toSimplePath(file));
    auto threadName = ThreadControl::getName(id);

    auto format = std_sprintf("{%%s  %%-10s  %%-%ds  %%-%ds  %%s}", length_group, length_name);
    return std_sprintf(format.c_str(), timeString, threadName, group, name, locationString);
}
void clear() {
    for(auto& e: all) {
       e.queue->clear();
    }
}
void dump(const std::string& group) {
    size_t size = 0;
    for(const auto& e: all) {
        size += e.queue->size();
    }
    std::vector<Event> result;
    result.reserve(size);
    for(const auto& e: all) {
        if (group == "" || group == e.group) {
            std::vector<Event> temp;
            std::merge(result.begin(), result.end(), e.queue->begin(), e.queue->end(), std::back_inserter(temp));
            result.clear();
            std::copy(temp.cbegin(), temp.cend(), std::back_inserter(result));
        }
    }
    int length_group = 0;
    int length_name  = 0;
    for(const auto& e: result) {
        int length;

        length = strlen(e.group);
        if (length_group < length) length_group = length;
        length = strlen(e.name);
        if (length_name< length) length_name = length;
    }
    for(const auto& e: result) {
        logger.info(e.toString(length_group, length_name));
    }
    logger.info("size  %d", size);
}

}
