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

static const constexpr bool TRACE_ENABLE = true;

#define TRACE_RECORD(group, name) { if (TRACE_ENABLE) { trace::Event event(#group, #name);  trace::group::name.push_back(event); }  }

namespace trace {

struct Event {
    const char*                           group;
    const char*                           name;
    std::chrono::system_clock::time_point time;
    std::source_location                  location;

    Event(
        const char* group_,
        const char* name_,
        std::chrono::system_clock::time_point time_ = std::chrono::system_clock::now(),
        std::source_location location_ = std::source_location::current()) :
        group(group_), name(name_), time(time_), location(location_) {}
    Event(Event&& that) = default;
    Event(const Event& that) = default;
    Event& operator =(const Event& that) = default;
    
    std::strong_ordering operator <=>(const Event& that) const {
        return this->time <=> that.time;
    }

    static inline std::strong_ordering comparator(const Event& left, const Event& right) {
        return left <=> right;
    }

    std::string toString() const;
};

inline constexpr int QUEUE_SIZE = 80;
using EventQueue = fixed_queue<Event, QUEUE_SIZE>;

struct Entry {
    std::string group;
    std::string name;
    EventQueue* queue;
    Entry(const char* group_, const char* name_, EventQueue* queue_) : group(group_), name(name_), queue(queue_) {}
};

extern std::vector<Entry> all;

void clear();
void dump(const std::string& group = "");

#define TRACE_DECLARE(group, name) namespace group { extern EventQueue name; }

// processor_thread
TRACE_DECLARE(processor, run)
TRACE_DECLARE(processor, requestRescheduleTimer)
TRACE_DECLARE(processor, requestRescheduleInterrupt)
TRACE_DECLARE(processor, checkRequestReschedule)
// interrupt_thread
TRACE_DECLARE(interrupt, run)
TRACE_DECLARE(interrupt, notifyInterrupt)
// timer_thread
TRACE_DECLARE(timer, run)
TRACE_DECLARE(timer, processTimeout)
// Opcode_process
TRACE_DECLARE(processor, reschedule)
}