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

#include <cinttypes>

#define PERF_DECLARE(group, name) namespace group { extern uint64_t name; }

#define PERF_COUNT(group, name) { if (PERF_ENABLE) perf::group::name++; }

#define PERF_LOG() { if (PERF_ENABLE) perf::dump(); }

static const bool PERF_ENABLE = true;

namespace perf {

void dump();

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
PERF_DECLARE(memory, GetAddress)
PERF_DECLARE(memory, FetchPda)
PERF_DECLARE(memory, StorePda)
PERF_DECLARE(memory, MemoryFetch)
PERF_DECLARE(memory, MemoryStore)

// opcode
PERF_DECLARE(opcode, Dispatch)
PERF_DECLARE(opcode, FrameFault)
PERF_DECLARE(opcode, PageFault)
PERF_DECLARE(opcode, CodeTrap)
PERF_DECLARE(opcode, EscOpcodeTrap)
PERF_DECLARE(opcode, OpcodeTrap)
PERF_DECLARE(opcode, UnboundTrap)

// running
PERF_DECLARE(running, start)
PERF_DECLARE(running, stop)

// interrupt
PERF_DECLARE(interrupt, notify)
PERF_DECLARE(interrupt, wakeup)
PERF_DECLARE(interrupt, interrupt)
PERF_DECLARE(interrupt, request)

// timer
PERF_DECLARE(timer, timer)
PERF_DECLARE(timer, timeout)
PERF_DECLARE(timer, updatePTC)

// processor
PERF_DECLARE(processor, abort)
PERF_DECLARE(processor, requestReschedule)
PERF_DECLARE(processor, needReschedule)
PERF_DECLARE(processor, checkRequestReschedule)
PERF_DECLARE(processor, rescheduleRequest)
PERF_DECLARE(processor, interruptFlag)
PERF_DECLARE(processor, interrupt)
PERF_DECLARE(processor, timerFlag)
PERF_DECLARE(processor, timer)
PERF_DECLARE(processor, running)

// network
PERF_DECLARE(network, transmit)
PERF_DECLARE(network, wait_for)
PERF_DECLARE(network, receive)
PERF_DECLARE(network, select)

// disk
PERF_DECLARE(disk, process)
PERF_DECLARE(disk, read)
PERF_DECLARE(disk, write)
PERF_DECLARE(disk, verify)

}
