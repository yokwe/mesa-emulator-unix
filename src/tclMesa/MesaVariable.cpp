/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

//
// MasaVariable.cpp
//

#include <tcl.h>
#include <tclDecls.h>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Variable.h"

int MesaVariable(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    (void)cdata; (void)objv;

    Tcl_Obj* result = 0;

    if (objc != 1) {
        result = Tcl_ObjPrintf("Unexpected objc is not equals to 1  objc = %d", objc);
        logger.error(Tcl_GetString(result));
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }
    // mesa::perf
    auto dict = Tcl_NewDictObj();

// // 3.3.2 Evaluation Stack
// VariableStack stack;
    {
        auto name = "stack";
        auto key = Tcl_NewStringObj(name, strlen(name));
        auto value = Tcl_NewListObj(0, 0);
        for(int i = 0; i < StackDepth; i++) {
            auto ret = Tcl_ListObjAppendElement(interp, value, Tcl_NewIntObj(stack[i]));
            if (ret != TCL_OK) ERROR()
        }
        int ret = Tcl_DictObjPut(interp, dict, key, value);
        if (ret != TCL_OK) ERROR()
    }
// CARD16 SP;
    putUINT16(interp, dict, "SP", SP);

// // 3.3.3 Data and Status Registers
// CARD16 PID[4]; // Processor ID
    {
        uint64_t pid = PID[1];
        pid <<= 16;
        pid |= PID[2];
        pid <<= 16;
        pid |= PID[3];

        put(interp, dict, "PID", std_sprintf("0x%lX", pid));
    }
// //extern CARD16 MP;     // Maintenance Panel
// VariableMP MP;
    putUINT16(interp, dict, "MP", (CARD16)MP);
// //extern CARD32 IT;     // Interval Timer
// VariableIT IT;

// //extern CARD16 WM;     // Wakeup mask register - 10.4.4
// //extern CARD16 WP;     // Wakeup pending register - 10.4.4.1
// VariableWP WP;
    put(interp, dict, "WP", std_sprintf("0x%04X", (CARD16)WP));
// //extern CARD16 WDC;    // Wakeup disable counter - 10.4.4.3
// VariableWDC WDC;
    putUINT16(interp, dict, "WDC", (CARD16)WDC);
// CARD16 PTC;    // Process timeout counter - 10.4.5
    put(interp, dict, "PTC", std_sprintf("0x%04X", PTC));
// CARD16 XTS;    // Xfer trap status - 9.5.5
    putUINT16(interp, dict, "XTS", XTS);
// // 3.3.1 Control Registers
// VariablePSB       PSB; // PsbIndex - 10.1.1
    putUINT16(interp, dict, "PSB", (CARD16)PSB);
// //extern MdsHandle         MDS;
// VariableMDS       MDS;
    put(interp, dict, "MDS", std_sprintf("0x%04X", ((CARD32)MDS) >> 16));
    put(interp, dict, "MDS", std_sprintf("0x%X", (CARD32)MDS));
// //LocalFrameHandle  LF;  // POINTER TO LocalVariables
// VariableLF        LF;
    put(interp, dict, "LF", std_sprintf("0x%04X", (CARD16)LF));
// //GlobalFrameHandle GF;  // LONG POINTER TO GlobalVarables
// VariableGF        GF;
    put(interp, dict, "GF", std_sprintf("0x%08X", (CARD32)GF));
// //CARD32            CB;  // LONG POINTER TO CodeSegment
// VariableCB        CB;
    put(interp, dict, "CB", std_sprintf("0x%08X", (CARD32)CB));
// CARD16            PC;
    put(interp, dict, "PC", std_sprintf("0x%04X", PC));
// GFTHandle         GFI;
    put(interp, dict, "GFI", std_sprintf("0x%04X", GFI));
// // 4.5 Instruction Execution
// CARD8  breakByte;
    put(interp, dict, "breakByte", std_sprintf("0x%02X", breakByte));
// CARD16 savedPC;
    put(interp, dict, "savedPC", std_sprintf("0x%04X", savedPC));
// CARD16 savedSP;
    putUINT16(interp, dict, "savedSP", savedSP);
// // 10.4.1 Scheduler
// VariableRunning running;
    putUINT16(interp, dict, "running", running ? 1 : 0);


    Tcl_SetObjResult(interp, dict);

    return TCL_OK;
}
