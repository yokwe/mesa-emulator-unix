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
#include "../util/GuiOp.h"

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
CARD16            PSB; // PsbIndex - 10.1.1
//extern MdsHandle         MDS;
LocalFrameHandle  LF;  // POINTER TO LocalVariables
GlobalFrameHandle GF;  // LONG POINTER TO GlobalVarables
CARD32            CB;  // LONG POINTER TO CodeSegment
CARD16            PC;
GFTHandle         GFI;

// 4.5 Instruction Execution
CARD8  breakByte;
CARD16 savedPC;
CARD16 savedSP;

// 10.4.1 Scheduler
VariableRunning running;


//
// VariableMP
//
static void mp_message(CARD16 mp) {
    logger.info("MP %04d", mp);
} 
void VariableMP::initialize() {
    observerList.push_back(mp_message);
    observerList.push_back(GuiOp::setMP);
    observerList.push_back(Logger::mp_observer);
    observerList.push_back(processor::mp_observer);
}
