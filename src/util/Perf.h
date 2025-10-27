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
// Perf.h
//

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define PERF_DECLARE(group, name) namespace group { extern uint64_t name; }

#define PERF_COUNT(group, name) { if (PERF_ENABLE) perf::group::name++; }
#define PERF_ADD(group, name, value) { if (PERF_ENABLE) perf::group::name += value; }

#define PERF_LOG() { if (PERF_ENABLE) perf::dump(); }
#define PERF_CLEAR() { perf::clear(); }

static const bool PERF_ENABLE = true;

namespace perf {

struct Entry {
    std::string group;
    std::string name;
    uint64_t&   value;
    Entry(const char* group_, const char* name_, uint64_t& value_) : group(group_), name(name_), value(value_) {}
};

extern std::vector<perf::Entry> all;

void dump();
void dump(const std::string& group);
void clear();

// memory
PERF_DECLARE(memory, Fetch)
PERF_DECLARE(memory, Store)
PERF_DECLARE(memory, ReadDbl)
PERF_DECLARE(memory, FetchMds)
PERF_DECLARE(memory, StoreMds)
PERF_DECLARE(memory, ReadDblMds)
PERF_DECLARE(memory, GetCodeByte)
PERF_DECLARE(memory, GetCodeWord)
PERF_DECLARE(memory, FetchByte)
PERF_DECLARE(memory, StoreByte)
PERF_DECLARE(memory, ReadField)
PERF_DECLARE(memory, WriteField)
PERF_DECLARE(memory, WriteMap)
PERF_DECLARE(memory, peek)
PERF_DECLARE(memory, FetchPda)
PERF_DECLARE(memory, StorePda)
PERF_DECLARE(memory, FetchPage)
PERF_DECLARE(memory, StorePage)

// opcode
PERF_DECLARE(opcode, Dispatch)
PERF_DECLARE(opcode, DispatchEsc)
PERF_DECLARE(opcode, FrameFault)
PERF_DECLARE(opcode, PageFault)
PERF_DECLARE(opcode, CodeTrap)
PERF_DECLARE(opcode, EscOpcodeTrap)
PERF_DECLARE(opcode, OpcodeTrap)
PERF_DECLARE(opcode, UnboundTrap)

// interrupt
PERF_DECLARE(interrupt, notifyInterrupt_ENTER)
PERF_DECLARE(interrupt, notifyInterrupt_EXIT)
PERF_DECLARE(interrupt, wakeup)
PERF_DECLARE(interrupt, interrupt)
PERF_DECLARE(interrupt, request)

// timer
PERF_DECLARE(timer, timer)
PERF_DECLARE(timer, processTimeout_ENTER)
PERF_DECLARE(timer, updatePTC)
PERF_DECLARE(timer, processTimeout_EXIT)

// processor
PERF_DECLARE(processor, abort)
PERF_DECLARE(processor, requestReschedule_ENTER)
PERF_DECLARE(processor, requestReschedule_EXIT)
PERF_DECLARE(processor, running_A_YES)
PERF_DECLARE(processor, running_A_NO)
PERF_DECLARE(processor, interruptFlag_YES)
PERF_DECLARE(processor, interruptFlag_NO)
PERF_DECLARE(processor, interrupt)
PERF_DECLARE(processor, timerFlag_YES)
PERF_DECLARE(processor, timerFlag_NO)
PERF_DECLARE(processor, timer)
PERF_DECLARE(processor, needReschedule_YES)
PERF_DECLARE(processor, needReschedule_NO)
PERF_DECLARE(processor, running_B_YES)
PERF_DECLARE(processor, running_B_NO)
PERF_DECLARE(processor, busyWait)
PERF_DECLARE(processor, checkRequestReschedule_ENTER)
PERF_DECLARE(processor, throw_RequestReschedule)
PERF_DECLARE(processor, checkRequestReschedule_EXIT)
PERF_DECLARE(processor, requestRescheduleTimer_ENTER)
PERF_DECLARE(processor, requestRescheduleTimer_EXIT)
PERF_DECLARE(processor, requestRescheduleInterrupt_ENTER)
PERF_DECLARE(processor, requestRescheduleInterrupt_EXIT)

// network
PERF_DECLARE(network, transmit)
PERF_DECLARE(network, receive_request)
PERF_DECLARE(network, receive_process)
PERF_DECLARE(network, receive_packet)

// disk
PERF_DECLARE(disk, process)
PERF_DECLARE(disk, read)
PERF_DECLARE(disk, write)
PERF_DECLARE(disk, verify)
PERF_DECLARE(disk, process_time)

// agent
PERF_DECLARE(agent, beep)
PERF_DECLARE(agent, disk)
PERF_DECLARE(agent, display)
PERF_DECLARE(agent, floppy)
PERF_DECLARE(agent, keyPress)
PERF_DECLARE(agent, keyRelease)
PERF_DECLARE(agent, mouse)
PERF_DECLARE(agent, network)
PERF_DECLARE(agent, processor)
PERF_DECLARE(agent, stream)

// variable
PERF_DECLARE(variable, MP)
PERF_DECLARE(variable, WDC)
PERF_DECLARE(variable, WDC_enable)
PERF_DECLARE(variable, WDC_disable)
PERF_DECLARE(variable, WP)
PERF_DECLARE(variable, WP_exchange)
PERF_DECLARE(variable, WP_fetch_or)
PERF_DECLARE(variable, IT)
PERF_DECLARE(variable, PSB)
PERF_DECLARE(variable, MDS)
PERF_DECLARE(variable, LF)
PERF_DECLARE(variable, GF)
PERF_DECLARE(variable, CB)
PERF_DECLARE(variable, running)
PERF_DECLARE(variable, running_start)
PERF_DECLARE(variable, running_stop)
PERF_DECLARE(variable, time_running)
PERF_DECLARE(variable, time_not_running)

// bpf
PERF_DECLARE(bpf, fillBuffer)
PERF_DECLARE(bpf, fillBuffer_data)
PERF_DECLARE(bpf, read)
PERF_DECLARE(bpf, read_empty)
PERF_DECLARE(bpf, read_select)
PERF_DECLARE(bpf, read_zero)

}
