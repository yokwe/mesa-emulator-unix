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
// Variable.cpp
//

//#include <map>

#include "Variable.h"

#include "../mesa/processor.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);


// 3.3.2 Evaluation Stack
VariableStack stack;
CARD16 SP;

// 3.3.3 Data and Status Registers
CARD16 PID[4]; // Processor ID

//extern CARD16 MP;     // Maintenance Panel
VariableMP MP;

//extern CARD32 IT;     // Interval Timer
VariableIT IT;

//extern CARD16 WM;     // Wakeup mask register - 10.4.4
//extern CARD16 WP;     // Wakeup pending register - 10.4.4.1
VariableWP WP;

//extern CARD16 WDC;    // Wakeup disable counter - 10.4.4.3
VariableWDC WDC;

CARD16 PTC;    // Process timeout counter - 10.4.5
CARD16 XTS;    // Xfer trap status - 9.5.5

// 3.3.1 Control Registers
VariablePSB       PSB; // PsbIndex - 10.1.1

//extern MdsHandle         MDS;
VariableMDS       MDS;

//LocalFrameHandle  LF;  // POINTER TO LocalVariables
VariableLF        LF;

//GlobalFrameHandle GF;  // LONG POINTER TO GlobalVarables
VariableGF        GF;

//CARD32            CB;  // LONG POINTER TO CodeSegment
VariableCB        CB;

CARD16            PC;
GFTHandle         GFI;

// 4.5 Instruction Execution
CARD8  breakByte;
CARD16 savedPC;
CARD16 savedSP;

// 10.4.1 Scheduler
VariableRunning running;


void variable::initialize() {
	// Processor ID
	PID[0] = 0x0000;
	PID[1] = 0x0000;
	PID[2] = 0x0000;
	PID[3] = 0x0000;
//	MP  = 0;    // Maintenance Panel
	MP.clear();
//	IT  = 0;    // Interval Timer
//	WM  = 0;    // Wakeup mask register - 10.4.4
//	WP  = 0;    // Wakeup pending register - 10.4.4.1
	WP = (CARD16)0;
//	WDC = 1;    // Wakeup disable counter - 10.4.4.3
	WDC = (CARD16)1;
	PTC = 0;    // Process timeout counter - 10.4.5
	XTS = 0;    // Xfer trap status - 9.5.5

	// 3.3.1 Control Registers
	PSB = (CARD16)0;  // PsbIndex - 10.1.1
	MDS = (CARD32)0;
	LF  = (CARD16)0;  // POINTER TO LocalVariables
	GF  = (CARD32)0;  // LONG POINTER TO GlobalVarables
	CB  = (CARD32)0;  // LONG POINTER TO CodeSegment
	PC  = 0;
	GFI = 0;

//	for(int i = 0; i < StackDepth; i++) stack[i] = 0;
//	SP = 0;
	stack.clear();

	// 4.5 Instruction Execution
	breakByte = 0;
	savedPC   = 0;
	savedSP   = 0;

	// 10.4.1 Scheduler
	//running = 1;
	running = true;

	// 10.4.5 Timeouts
    //lastTimeoutTime = 0;
}


//
// VariableMP
//
static void mp_message(CARD16 mp) {
    logger.info("MP %04d", mp);
} 
void VariableMP::initialize() {
	addObserver(mp_message);
    addObserver(Logger::mp_observer);
    addObserver(processor::mp_observer);
}
