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
// Variable.cpp
//

#include "Type.h"
#include "PrincOps.h"
#include "Variable.h"

namespace mesa {

// 3.3.1 Control Registers
CARD16            PSB = 0; // PsbIndex - 10.1.1
MdsHandle         MDS = 0;
LocalFrameHandle  LF  = 0;  // POINTER TO LocalVariables
GlobalFrameHandle GF  = 0;  // LONG POINTER TO GlobalVarables
CARD32            CB  = 0;  // LONG POINTER TO CodeSegment
CARD16            PC  = 0;
GFTHandle         GFI = 0;

// 3.3.2 Evaluation Stack
CARD16 stack[StackDepth];
CARD16 SP = 0;

// 3.3.3 Data and Status Registers
CARD16 PID[4] = {0, 0, 0, 0}; // Processor ID
//CARD16 MP  = 0;    // Maintenance Panel
//CARD32 IT  = 0;    // Interval Timer
//CARD16 WM  = 0;    // Wakeup mask register - 10.4.4
//CARD16 WP  = 0;    // Wakeup pending register - 10.4.4.1
//CARD16 WDC = 0;    // Wakeup disable counter - 10.4.4.3
//CARD16 PTC = 0;    // Process timeout counter - 10.4.5
CARD16 XTS = 0;    // Xfer trap status - 9.5.5


// 4.5 Instruction Execution
CARD8  breakByte = 0;
CARD16 savedPC = 0;
CARD16 savedSP = 0;


void VariableInit() {
	// Processor ID
	PID[0] = 0x0000;
	PID[1] = 0x0000;
	PID[2] = 0x0000;
	PID[3] = 0x0000;
//	MP  = 0;    // Maintenance Panel
// FIXME	ProcessorThread::setMP(0);
//	IT  = 0;    // Interval Timer
//	WM  = 0;    // Wakeup mask register - 10.4.4
//	WP  = 0;    // Wakeup pending register - 10.4.4.1
// FIXME	InterruptThread::setWP(0);
//	WDC = 1;    // Wakeup disable counter - 10.4.4.3
// FIXME	InterruptThread::setWDC(1);
//	PTC = 0;    // Process timeout counter - 10.4.5
// FIXME	TimerThread::setPTC(0);
	XTS = 0;    // Xfer trap status - 9.5.5

	// 3.3.1 Control Registers
	PSB = 0; // PsbIndex - 10.1.1
	MDS = 0;
	LF  = 0;  // POINTER TO LocalVariables
	GF  = 0;  // LONG POINTER TO GlobalVarables
	CB  = 0;  // LONG POINTER TO CodeSegment
	PC  = 0;
	GFI = 0;

	for(int i = 0; i < StackDepth; i++) stack[i] = 0;
	SP = 0;

	// 4.5 Instruction Execution
	breakByte = 0;
	savedPC   = 0;
	savedSP   = 0;

	// 10.4.1 Scheduler
	//running = 1;
	// FIXME ProcessorThread::startRunning();

	// 10.4.5 Timeouts
    // FIXME lastTimeoutTime = 0;
}

}
