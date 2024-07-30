/*******************************************************************************
 * Copyright (c) 2024, Yasuhiro Hasegawa
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

#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define PPSTR_NX(A) #A
#define PPSTR(A) PPSTR_NX(A)

#define PERF_NAME(name) PPCAT(perf_, name)

#define PERF_DECLARE(name) extern std::uint64_t PERF_NAME(name);
#define PERF_DEFFINE(name) std::uint64_t PERF_NAME(name) = 0;

#define PERF_COUNT(name) { if (PERF_ENABLE) PERF_NAME(name)++; }

#define PERF_DUMP(name) {if (PERF_NAME(name)) logger.info("%-22s = %10llu", PPSTR(PERF_NAME(name)), PERF_NAME(name));}

static const int PERF_ENABLE    = 1;

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


#define PERF_LOG() \
if (PERF_ENABLE) { \
	PERF_DUMP(Dispatch) \
	PERF_DUMP(Fetch) \
	PERF_DUMP(Store) \
	PERF_DUMP(ReadDbl) \
	PERF_DUMP(FetchMds) \
	PERF_DUMP(StoreMds) \
	PERF_DUMP(ReadDblMds) \
	PERF_DUMP(GetCodeByte) \
	PERF_DUMP(GetCodeWord) \
	PERF_DUMP(FetchByte) \
	PERF_DUMP(StoreByte) \
	PERF_DUMP(ReadField) \
	PERF_DUMP(WriteField) \
	PERF_DUMP(WriteMap) \
	PERF_DUMP(GetAddress) \
	PERF_DUMP(FetchPda) \
	PERF_DUMP(StorePda) \
	PERF_DUMP(MemoryFetch) \
	PERF_DUMP(MemoryStore) \
	PERF_DUMP(FrameFault) \
	PERF_DUMP(PageFault) \
	PERF_DUMP(CodeTrap) \
	PERF_DUMP(EscOpcodeTrap) \
	PERF_DUMP(OpcodeTrap) \
	PERF_DUMP(UnboundTrap) \
}
