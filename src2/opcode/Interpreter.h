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
// Interpreter.h
//

#pragma once

#include "Opcode.h"

#include "../mesa/Perf.h"

namespace mesa {

class Interpreter {
public:
	static const int TABLE_SIZE = 256;

	inline void dispatchEsc(CARD32 opcode) {
		// ESC and ESCL
		//logger.debug("dispatch ESC  %04X opcode = %03o", savedPC, opcode);
		tableEsc[opcode].execute();
		// increment stat counter after execution. We don't count ABORTED instruction.
		if (DEBUG_SHOW_OPCODE_STATS) statEsc[opcode]++;
	}

	inline void dispatchMop(CARD32 opcode) {
		PERF_COUNT(Dispatch)
		tableMop[opcode].execute();
		// increment stat counter after execution. We don't count ABORTED instruction.
		if (DEBUG_SHOW_OPCODE_STATS) statMop[opcode]++;
	}

	inline void execute() {
		savedPC = PC;
		savedSP = SP;
		dispatchMop(GetCodeByte());
	}

	// Implementation Specific
	void initialize();

	void stats();

private:
	Opcode    tableMop[TABLE_SIZE];
	Opcode    tableEsc[TABLE_SIZE];
	long long statMop [TABLE_SIZE];
	long long statEsc [TABLE_SIZE];

	void initRegisters();

	void initTable();
	void fillOpcodeTrap();

	static void mopOpcodeTrap();
	static void escOpcodeTrap();

	void assignMop(Opcode::EXEC exec_, const std::string name_, CARD32 code_, CARD32 size_);
	void assignEsc(Opcode::EXEC exec_, const std::string name_, CARD32 code_, CARD32 size_);
};

extern Interpreter interpreter;

static inline void Execute() {
	interpreter.execute();
}

}
