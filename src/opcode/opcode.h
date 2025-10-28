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
// opcode.h
//

#pragma once

#include <array>
#include <string>

#include "../mesa/Variable.h"
#include "../mesa/memory.h"

#include "../util/Debug.h"

namespace opcode {

// opcode name
static const int TABLE_SIZE = 256;

// name
extern std::array<std::string, TABLE_SIZE> nameMop;
extern std::array<std::string, TABLE_SIZE> nameEsc;

// op
typedef void (*op)();
extern std::array<op, TABLE_SIZE> opMop;
extern std::array<op, TABLE_SIZE> opEsc;

// stats
extern std::array<uint64_t, TABLE_SIZE> statsMop;
extern std::array<uint64_t, TABLE_SIZE> statsEsc;

// last
extern int lastMop;
extern int lastEsc;

void initialize();
void stats();

std::string lastOpcodeName();

}

#define OPCODE_STATS_COUNT_BEFORE 

inline void DispatchEsc(uint8_t code) {
    PERF_COUNT(opcode, DispatchEsc)
#ifdef OPCODE_STATS_COUNT_BEFORE
	// increment stat counter before execution for opcode that generate EscOpcodeTrap
	if (DEBUG_SHOW_OPCODE_STATS) opcode::statsEsc[code]++;
#endif
    opcode::lastEsc = code;
    opcode::opEsc[code]();
    opcode::lastEsc = -1;
#ifndef OPCODE_STATS_COUNT_BEFORE
	// increment stat counter after execution. We don't count ABORTED instruction.
	if (DEBUG_SHOW_OPCODE_STATS) opcode::statsEsc[code]++;
#endif
}

// 4.5 Instruction Execution
inline void Dispatch(uint8_t code) {
    PERF_COUNT(opcode, Dispatch)
#ifdef OPCODE_STATS_COUNT_BEFORE
	// increment stat counter before execution for opcode that generate OpcodeTrap
	if (DEBUG_SHOW_OPCODE_STATS) opcode::statsMop[code]++;
#endif
    opcode::lastMop = code;
    opcode::opMop[code]();
    opcode::lastMop = -1;
#ifndef OPCODE_STATS_COUNT_BEFORE
	// increment stat counter after execution. We don't count ABORTED instruction.
	if (DEBUG_SHOW_OPCODE_STATS) opcode::statsMop[code]++;
#endif
}
inline void Execute() {
    savedPC = PC;
    savedSP = SP;
    Dispatch(GetCodeByte());
}


//
// declaration of opcode function
//
#undef MOP
#undef ESC
#define MOP(enable, code, prefix, name) extern void E_##name();
#define ESC(enable, code, prefix, name) extern void E_##name();

#include "opcode.inc"
