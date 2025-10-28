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
// OpcodeTable.cpp
//

#include <array>
#include <cstring>
#include <cstdlib>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "opcode.h"

namespace opcode {

// name
std::array<std::string, TABLE_SIZE> nameMop;
std::array<std::string, TABLE_SIZE> nameEsc;

// op
std::array<op, TABLE_SIZE> opMop;
std::array<op, TABLE_SIZE> opEsc;

// stats
std::array<uint64_t, TABLE_SIZE> statsMop;
std::array<uint64_t, TABLE_SIZE> statsEsc;

// last
int lastMop;
int lastEsc;

// forward declaration
static void registerOpcode();

static void mopOpcodeTrap() {
	if (lastMop == 0) ERROR();
	OpcodeTrap(lastMop);
}
static void escOpcodeTrap() {
	if (lastEsc == 0) ERROR();
	EscOpcodeTrap(lastEsc);
}


void initialize() {
    nameMop.fill("");
    nameEsc.fill("");
    opMop.fill(nullptr);
    opMop.fill(nullptr);
    statsMop.fill(0);
    statsEsc.fill(0);
    lastMop = -1;
    lastEsc = -1;
    
    registerOpcode();
    
    // supply name to uninitialized entry
    for(int i = 0; i < TABLE_SIZE; i++) {
        if (nameMop[i].empty()) nameMop[i] = std_sprintf("mop-%03o", i);
        if (nameEsc[i].empty()) nameEsc[i] = std_sprintf("esc-%03o", i);
    }

    // supply opcodeTrap to uninitialized entry
    for(int i = 0; i < TABLE_SIZE; i++) {
        if (opMop[i] == nullptr) opMop[i] = mopOpcodeTrap;
        if (opEsc[i] == nullptr) opEsc[i] = escOpcodeTrap;
    }
}

void stats() {
	if (DEBUG_SHOW_OPCODE_STATS) {
		uint64_t total = 0;
		logger.info("==== opcode stats  START");
		for(int i = 0; i < TABLE_SIZE; i++) {
            bool opIsTrap = opMop[i] == mopOpcodeTrap;
			if (statsMop[i] == 0 && opIsTrap) continue;
            auto number = formatWithCommas(statsMop[i]);
			logger.info("stats mop  %3o  %-12s  %10s %s", i, nameMop[i], number, opIsTrap ? "*" : "");
			total += statsMop[i];
		}
		for(int i = 0; i < TABLE_SIZE; i++) {
            bool opIsTrap = opEsc[i] == escOpcodeTrap;
			if (statsEsc[i] == 0 && opIsTrap) continue;
            auto number = formatWithCommas(statsEsc[i]);
			logger.info("stats esc  %3o  %-12s  %10s %s", i, nameEsc[i], number, opIsTrap ? "*" : "");
			total += statsEsc[i];
		}
		logger.info("total = %s", formatWithCommas(total));
		logger.info("==== opcode stats  STOP");
	}
}

std::string lastOpcodeName() {
    if (lastEsc == -1 && lastMop == -1) return "*NONE*";
    if (lastEsc != -1) return nameEsc[lastEsc];
    if (lastMop != -1) return nameMop[lastMop];
    ERROR();
}

//
// registration of opcode
//
static void registerMop(int enable, int code_, op op_, const char* name_) {
    if (enable) opMop.at(code_) = op_;
    nameMop.at(code_) = name_;
}
static void registerEsc(int enable, int code_, op op_, const char* name_) {
    if (enable) opEsc.at(code_) = op_;
    nameEsc.at(code_) = name_;
}

#undef MOP
#undef ESC
#define MOP(enable, code, prefix, name) registerMop(enable, prefix##name, E_##name, #name); 
#define ESC(enable, code, prefix, name) registerEsc(enable, prefix##name, E_##name, #name); 

static void registerOpcode() {
#include "opcode.inc"
}

}