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
#include <deque>
#include <mutex>
#include <vector>

static const constexpr bool TRACE_ENABLE = false;

#define TRACE_REC_(group, name) { if (TRACE_ENABLE) { trace::Event event(#group, #name, __FILE__, __LINE__);  trace::group.push_back(event); } }

namespace trace {

// https://stackoverflow.com/questions/56334492/c-create-fixed-size-queue
template <typename T, int MAX_SIZE>
class fixed_queue : public std::deque<T> {
    // needs mutex for using under multi threads
	std::mutex mutex;
public:
	fixed_queue() : std::deque<T>() {}
	fixed_queue(const fixed_queue& that) = default;

	void pop_back() {
		std::unique_lock<std::mutex> lock(mutex);
		std::deque<T>::pop_back();
	}
	void pop_front() {
		std::deque<T>::pop_front();
	}

    void push_front(const T& value) {
		std::unique_lock<std::mutex> lock(mutex);
        if (this->size() == MAX_SIZE) {
           std::deque<T>::pop_back();
        }
        std::deque<T>::push_front(value);
    }
    void push_back(const T& value) {
		std::unique_lock<std::mutex> lock(mutex);
        if (this->size() == MAX_SIZE) {
           std::deque<T>::pop_front();
        }
        std::deque<T>::push_back(value);
    }
};


struct Event {
    const char*                           group;
    const char*                           name;
    const char*                           file;
    uint32_t                              line;
    std::chrono::system_clock::time_point time;

    Event(
        const char* group_,
        const char* name_,
        const char* file_,
        uint32_t    line_,
        std::chrono::system_clock::time_point time_ = std::chrono::system_clock::now()) :
            group(group_), name(name_), file(file_), line(line_), time(time_) {}
    // Event(Event&& that) = default;
    // Event(const Event& that) = default;
    // Event& operator =(const Event& that) = default;
    
    std::strong_ordering operator <=>(const Event& that) const {
        return this->time <=> that.time;
    }
    bool operator < (const Event& that) const {
        return this->time < that.time;
    }

    // static inline std::strong_ordering comparator(const Event& left, const Event& right) {
    //     return left <=> right;
    // }

    std::string toString(int length_group = 9, int length_name = 26) const;
};

inline constexpr int QUEUE_SIZE = 100000;
using EventQueue = fixed_queue<Event, QUEUE_SIZE>;

struct Entry {
    std::string group;
    EventQueue* queue;
    Entry(const char* group_, EventQueue* queue_) : group(group_), queue(queue_) {}
};

extern std::vector<Entry> all;

void clear();
void dump(const std::string& group = "");

#define TRACE_DECLARE(group) extern EventQueue group;

// processor_thread
TRACE_DECLARE(processor)
// interrupt_thread
TRACE_DECLARE(interrupt)
// timer_thread
TRACE_DECLARE(timer)

}