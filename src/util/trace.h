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
// trace.h
//

#pragma once

#include <chrono>
#include <source_location>

#include "Util.h"

#define TRACE_RECORD(name) { Event event; trace::group::name.push(event); }

namespace trace {

struct Event {
    std::chrono::system_clock::time_point time;
    std::source_location                  location;

    Event(
        std::chrono::system_clock::time_point time_ = std::chrono::system_clock::now(),
        std::source_location location_ = std::source_location::current()) :
        time(time_), location(location_) {}
};

inline constexpr int QUEUE_SIZE = 20;
using EventQueue = fixed_queue<Event, QUEUE_SIZE>;

extern std::map<const char*, EventQueue*> map;

void clear();
void dump(const char* name);

#define TRACE_DECLARE(name) namespace group { extern EventQueue name; }

TRACE_DECLARE(guam)
TRACE_DECLARE(processor)
TRACE_DECLARE(interrupt)
TRACE_DECLARE(timer)

}