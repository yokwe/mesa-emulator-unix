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

#include <map>

#include "Variable.h"

#include "../mesa/ProcessorThread.h"
#include "../util/GuiOp.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);


VariableMP MP;

// static std::map<CARD16, const char*> mp_message_map = {
//     {900, "Germ entered"},
//     {901, "Germ out of frames (Pilot bug)"},
//     {902, "unexpected trap or kernel function call (Pilot bug)"},
//     {903, "attempt to start an already started module (Pilot bug)"},
//     {904, "page or write protect fault encountered (Pilot bug)"},
//     {905, "Germ not compatible with initial microcode"},
//     {906, "Germ and running Pilot have different version numbers"},
//     {907, "reschedule error, typically because of page or frame fault (Pilot bug)"},
//     {909, "Germ SIGNAL or ERROR (Pilot bug)"},
//     {910, "Germ action running (e.g. inLoad, outLoad)"},
//     {911, "Germ and physical volume have incompatible version numbers"},
//     {912, "Germ and boot file have incompatible version numbers"},
//     {913, "no physical boot file installed"},
//     {914, "boot file contains invalid data"},
//     {915, "waiting for ethernet debugger to begin debugging me"},
//     {916, "boot file won't fit in real memory"},
//     {917, "talking to ethernet debugger"},
//     {919, "Germ transferred control back to caller (who has hung)"},
//     {920, "Germ driver running (e.g. disk, ether, floppy)"},
//     {921, "hard error on device being booted"},
//     {922, "operation on boot device no completed in expected time"},
//     {923, "broken link in chained boot file (try reinstalling)"},
//     {924, "no response to Germ's request for ether boot file"},
//     {925, "e.g. unexpected sequence number or size"},
//     {926, "booting media needs attention, e.g., retentioning"},
//     {927, "boot file ends before it should (try reinstalling)"},
//     {928, "waiting for any boot server to respond"},
//     {929, "expected descriptor page doesn't look like one (try reinstalling)"},
//     {930, "Pilot Control and MesaRuntime components being initialized"},
//     {931, "Pilot and StartPilot have incompatible version numbers"},
//     {932, "runtime trap before appropriate trap handler set up (Pilot bug)"},
//     {933, "Pilot and Germ have incompatible version numbers"},
//     {934, "boot file's StartList contains bad data"},
//     {935, "need ethernet debugee server but boot loader being used does not have that capability."},
//     {936, "waiting for microcode debugger"},
//     {937, "trying to get the time from either hardware clock or ethernet"},
//     {938, "running cleanup procedures, e.g. before going to debugger"},
//     {939, "ProcessorFace.PowerOff called but no power control relay"},
//     {940, "Pilot Store component being initialized"},
//     {941, "Bad LoadState version"},
//     {946, "system logical volume needs scavenging[riskyRepair]"},
//     {947, "waiting for disk drive to become ready"},
//     {948, "system physical volume needs scavenging"},
//     {949, "disk hardware error while scavenging system volume"},
//     {950, "logical volume being scavenged"},
//     {951, "alternate feedback for progress during a pass of logical volume scavenging"},
//     {952, "alternate feedback for additional passes during logical volume scavenging"},
//     {953, "debugger pointers have been set to a nonexistent volume or to a volume without an installed debugger"},
//     {960, "temporary files from previous run being deleted"},
//     {965, "insufficient file space for data space backing storage (specify smaller size with boot switch)"},
//     {966, "insufficient file space for file lock nodes"},
//     {970, "client and other non-bootloaded code being mapped"},
//     {980, "Pilot Communication component being initialized"},
//     {981, "trying to find a Pup / EthernetOne 8 bit address"},
//     {982, "can't determine ARPA 32 bit host address"},
//     {990, "PilotClient.Run called"},
// };

static void mp_message(CARD16 mp) {
    // std::string message = mp_message_map.contains(mp) ? mp_message_map.at(mp) : "";
    // logger.info("MP %04d %s", mp, message);
    logger.info("MP %04d", mp);
} 

void VariableMP::addObserver() {
    addSetObserver(mp_message);
    addSetObserver(GuiOp::setMP);
    addSetObserver(Logger::mp_observer);
    addSetObserver(ProcessorThread::mp_observer);
}
