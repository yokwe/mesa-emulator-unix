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
// Constant.h
//

#pragma once

#include "MesaBasic.h"

// for compatibility
//const CARD16 PageSize = Environment::wordsPerPage;
const int PageSize   = 256;
const int PageOffset = 0xFF;

const CARD16 cSS = 14;
// 3.3.2 Evaluation Stack
const int StackDepth = cSS;


// cSV = SIZE[StateVector] + MAX[SIZE[ControlLink], SIZE[FSIndex], SIZE[LONG POINTER]]
// SIZE[StateVector] = 16 and SIZE[LONG POINTER] = 2. So cSV should be 16 + 2 = 18
const CARD16 cSV = 18;
//const CARD16 cWM = 10; // Noone use this value - cWDC <= cWM
const CARD16 cWDC = 7;
//const CARD16 cTickMin = 15;
//const CARD16 cTickMax = 60;

// 1 tick = 40 milliseconds
const CARD16 cTick = 40;

const CARD32 mPDA = 0x00010000; // 0200000
const CARD32 mGFT = 0x00020000; // 0400000

const CARD16 mAV  = 0x0100; //  0400
const CARD16 mSD  = 0x0200; // 01000
const CARD16 mETT = 0x0400; // 02000

const CARD16 qFrameFault        = 0;
const CARD16 qPageFault         = 1;
const CARD16 qWriteProtectFault = 2;

const CARD16 sBoot            = 001; //  1
const CARD16 sBoundsTrap      = 016; // 14
const CARD16 sBreakTrap       = 000; //  0
const CARD16 sCodeTrap        = 007; //  7
const CARD16 sControlTrap     = 006; //  6
const CARD16 sDivCheckTrap    = 013; // 11
const CARD16 sDivZeroTrap     = 012; // 10
const CARD16 sInterruptError  = 014; // 12
const CARD16 sOpcodeTrap      = 005; //  5
const CARD16 sPointerTrap     = 017; // 15
const CARD16 sProcessTrap     = 015; // 13
const CARD16 sRescheduleError = 003; //  3
const CARD16 sStackError      = 002; //  2
const CARD16 sUnboundTrap     = 011; //  9
const CARD16 sXferTrap        = 004; //  4
const CARD16 sHardwareError   = 010; //  8


// Instruction code
#undef MOP
#undef ESC
#define MOP(enable, code, prefix, name) const CARD8 prefix##name = code;
#define ESC(enable, code, prefix, name) const CARD8 prefix##name = code;

#include "../opcode/opcode.inc"
