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

#include "Util.h"
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
static const Logger logger(__FILE__);

#include "trace.h"

namespace trace {

#include "trace.inc"

static const char* toSimplePath(const char* path) {
    auto pos = strstr(path, "/src");
    return (pos == NULL) ? path : pos + 1;
}
static std::string toStringLocalTime(const std::chrono::system_clock::time_point time) {
    time_t temp = std::chrono::system_clock::to_time_t(time);
	auto microsecond = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1'000'000;

    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%d-%02d-%02d %02d:%02d:%02d.%06d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, microsecond);
}
std::string Event::toString() const {
    auto timeString = toStringLocalTime(time);
    auto nameString = std_sprintf("%s::%s", group, name);
    auto locationString = std_sprintf("%5d  %s", location.line(), toSimplePath(location.file_name()));
    return std_sprintf("{%s  %-38s  %s}", timeString, nameString, locationString);
}
void clear() {
    for(auto& e: all) {
       e.queue->clear();
    }
}
void dump(const std::string& group) {
    std::vector<Event> vector;
    for(const auto& e: all) {
        if (group == "" || group == e.group) {
            std::copy(e.queue->cbegin(), e.queue->cend(), std::back_inserter(vector));
        }
    }
    std::sort(vector.begin(), vector.end(), std::less<Event>{});
    for(const auto& e: vector) {
        logger.info(e.toString());
    }
}

}
