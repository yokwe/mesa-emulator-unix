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

#include <unordered_map>

#include "Variable.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/processor.h"

#include "../opcode/opcode.h"


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

void variable::Values::set() {
	for(int i = 0; i < 4; i++) PID[i] = ::PID[i];
	MP  = ::MP;
	WP  = ::WP;
	WDC = ::WDC;
	PTC = ::PTC;
	XTS = ::XTS;
	PSB = ::PSB;
	MDS = ::MDS;
	LF  = ::LF;
	GF  = ::GF;
	CB  = ::CB;
	GFI = ::GFI;
	PC  = ::PC;
	SP  = ::SP;
	savedPC   = ::savedPC;
	savedSP   = ::savedSP;
	for(int i = 0; i < StackDepth; i++) stack[i] = ::stack[i];
	breakByte = ::breakByte;
	running   = ::running;

	lastOpcode = opcode::lastOpcodeName();
}

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

void variable::dump() {
	Values values;
	values.set();

	std::vector<std::pair<std::string, std::string>> output;

	output.push_back(std::make_pair("PID", std_sprintf("0x%lX", (uint64_t)values.PID[1] << 32 | values.PID[2] << 16 | values.PID[3])));
	output.push_back(std::make_pair("WP", std_sprintf("0x%04X", values.WP)));

	output.push_back(std::make_pair("WDC", std_sprintf("%d", values.WDC)));
	output.push_back(std::make_pair("PTC", std_sprintf("0x%04X", values.PTC)));
	output.push_back(std::make_pair("XTS", std_sprintf("%d", values.XTS)));
	output.push_back(std::make_pair("PSB", std_sprintf("%d", values.PSB)));
	output.push_back(std::make_pair("MDS", std_sprintf("0x%04X", values.MDS >> 16)));
	output.push_back(std::make_pair("LF", std_sprintf("0x%04X", values.LF)));
	output.push_back(std::make_pair("GF", std_sprintf("0x%08X", values.GF)));
	output.push_back(std::make_pair("CB", std_sprintf("0x%08X", values.CB)));
	output.push_back(std::make_pair("GFI", std_sprintf("0x%04X", values.GFI)));
	output.push_back(std::make_pair("PC", std_sprintf("0x%04X", values.PC)));
	output.push_back(std::make_pair("SP", std_sprintf("%d", values.SP)));
	output.push_back(std::make_pair("savedPC", std_sprintf("0x%04X", values.savedPC)));
	output.push_back(std::make_pair("savedSP", std_sprintf("%d", values.savedSP)));

	{
		std::string string;
		for(int i = 0; i < StackDepth; i++) {
			string += std_sprintf(" 0x%04X", values.stack[i]);
		}
		output.push_back(std::make_pair("stack", string.substr(1)));
	}

	output.push_back(std::make_pair("breakByte", std_sprintf("0x%02X", values.breakByte)));
	output.push_back(std::make_pair("running", values.running ? "1" : "0"));
	output.push_back(std::make_pair("lastOpcode", values.lastOpcode));

	size_t firstLen  = 0;
	size_t secondLen = 0;
	for(auto& e: output) {
		firstLen  = std::max(firstLen, e.first.length());
		secondLen = std::max(secondLen, e.second.length());
	}
	secondLen = 14;
	std::string format = std_sprintf("%%-%ds = %%%ds", firstLen, secondLen);
	for(auto& e: output) {
		logger.info(format.c_str(), e.first, e.second);
	}
}


//
// VariableMP
//
static std::unordered_map<CARD16, const char*> mp_message_map = {
    {900, "Germ entered"},
    {901, "Germ out of frames (Pilot bug)"},
    {902, "unexpected trap or kernel function call (Pilot bug)"},
    {903, "attempt to start an already started module (Pilot bug)"},
    {904, "page or write protect fault encountered (Pilot bug)"},
    {905, "Germ not compatible with initial microcode"},
    {906, "Germ and running Pilot have different version numbers"},
    {907, "reschedule error, typically because of page or frame fault (Pilot bug)"},
    {909, "Germ SIGNAL or ERROR (Pilot bug)"},
    {910, "Germ action running"}, // inLoad outLoad
    {911, "Germ and physical volume have incompatible version numbers"},
    {912, "Germ and boot file have incompatible version numbers"},
    {913, "no physical boot file installed"},
    {914, "boot file contains invalid data"},
    {915, "waiting for ethernet debugger to begin debugging me"},
    {916, "boot file won't fit in real memory"},
    {917, "talking to ethernet debugger"},
    {919, "Germ transferred control back to caller (who has hung)"},
    {920, "Germ driver running"}, // disk, ether, floppy
    {921, "hard error on device being booted"},
    {922, "operation on boot device no completed in expected time"},
    {923, "broken link in chained boot file (try reinstalling)"},
    {924, "no response to Germ's request for ether boot file"},
    {925, "e.g. unexpected sequence number or size"},
    {926, "booting media needs attention, e.g., retentioning"},
    {927, "boot file ends before it should (try reinstalling)"},
    {928, "waiting for any boot server to respond"},
    {929, "expected descriptor page doesn't look like one (try reinstalling)"},
    {930, "Pilot Control and MesaRuntime components being initialized"},
    {931, "Pilot and StartPilot have incompatible version numbers"},
    {932, "runtime trap before appropriate trap handler set up (Pilot bug)"},
    {933, "Pilot and Germ have incompatible version numbers"},
    {934, "boot file's StartList contains bad data"},
    {935, "need ethernet debugee server but boot loader being used does not have that capability."},
    {936, "waiting for microcode debugger"},
    {937, "trying to get the time from either hardware clock or ethernet"},
    {938, "running cleanup procedures, e.g. before going to debugger"},
    {939, "ProcessorFace.PowerOff called but no power control relay"},
    {940, "Pilot Store component being initialized"},
    {941, "Bad LoadState version"},
    {946, "system logical volume needs scavenging[riskyRepair]"},
    {947, "waiting for disk drive to become ready"},
    {948, "system physical volume needs scavenging"},
    {949, "disk hardware error while scavenging system volume"},
    {950, "logical volume being scavenged"},
    {951, "alternate feedback for progress during a pass of logical volume scavenging"},
    {952, "alternate feedback for additional passes during logical volume scavenging"},
    {953, "debugger pointers have been set to a nonexistent volume or to a volume without an installed debugger"},
    {960, "temporary files from previous run being deleted"},
    {965, "insufficient file space for data space backing storage (specify smaller size with boot switch)"},
    {966, "insufficient file space for file lock nodes"},
    {970, "client and other non-bootloaded code being mapped"},
    {980, "Pilot Communication component being initialized"},
    {981, "trying to find a Pup / EthernetOne 8 bit address"},
    {982, "can't determine ARPA 32 bit host address"},
    {990, "PilotClient.Run called"},
	// ViewPoint Application Developers Guide
	// C.1 ViewPoint Savenger Maintenace Panel Codes
	{7500, "Scavenge in progress. No action required"},
	{7501, "User volume requires scavenging . Press the F and C keys to proceed"},
	{7502, "Appears afer you press F and C from 7501, should to to 7500 when you release F and C"},
	{7504, "Volume needs initializing. Press and release I and V keys"},
	{7508, "Volume needs converting Press and release F and C keys to proceed"},
	{7511, "There is no scavenger boot file on the Scavenger, System or Star volume"},
	// C.1.1 Normal MP Codes
	{7920, "Start ViewPoint"},
	{7940, "Start invisible application folders"},
	{7960, "Start visible application folder with autorun = TRUE"},
	{8000, "Done booting"},

	{7600, "Start ViewPoint"},
	{7700, "Start invisible application folders"},
	{7800, "Start visible application folder with autorun = TRUE"},

	// C1.2 Debugger Substitute MP Codes
	{7540, "AddressFault"},
	{7541, "BreadPoint"},
	{7542, "Bug"},
	{7543, "CallDebugger"},
	{7544, "CleanMapLog"},
	{7545, "Diskerror"},
	{7546, "Interrupt"},
	{7547, "Return"},
	{7548, "ReturnAborted"},
	{7549, "UncaughtSignal"},
	{7550, "VisitDebugger"},
	{7551, "WriteProtectFault"},
	{7552, "Other"},
};
static void mp_message(CARD16 mp) {
	if (mp_message_map.contains(mp)) {
		logger.info("MP %04d  %s", mp, mp_message_map.at(mp));
	} else {
		logger.info("MP %04d", mp);
	}
} 
void VariableMP::initialize() {
	addObserver(mp_message);
    addObserver(Logger::mp_observer);
    addObserver(processor::mp_observer);
}
