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

#define PERF_DECLARE(name) extern uint64_t name;

#define PERF_COUNT(name) { if (PERF_ENABLE) perf::name++; }

#define PERF_LOG() { if (PERF_ENABLE) perf::dump(); }

static const bool PERF_ENABLE = true;

namespace perf {

void dump();

PERF_DECLARE(Dispatch)
PERF_DECLARE(Fetch)
PERF_DECLARE(Store)
PERF_DECLARE(ReadDbl)
PERF_DECLARE(FetchMds)
PERF_DECLARE(StoreMds)
PERF_DECLARE(ReadDblMds)
PERF_DECLARE(GetCodeByte)
PERF_DECLARE(GetCodeWord)
PERF_DECLARE(FetchByte)
PERF_DECLARE(StoreByte)
PERF_DECLARE(ReadField)
PERF_DECLARE(WriteField)
PERF_DECLARE(WriteMap)
PERF_DECLARE(GetAddress)
PERF_DECLARE(FetchPda)
PERF_DECLARE(StorePda)
PERF_DECLARE(MemoryFetch)
PERF_DECLARE(MemoryStore)
// Fault
PERF_DECLARE(FrameFault)
PERF_DECLARE(PageFault)
// Trap
PERF_DECLARE(CodeTrap)
PERF_DECLARE(EscOpcodeTrap)
PERF_DECLARE(OpcodeTrap)
PERF_DECLARE(UnboundTrap)

}
