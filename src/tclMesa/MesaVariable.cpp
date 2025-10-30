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
#include <utility>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Variable.h"

#include "../opcode/opcode.h"

#include "../util/tcl.h"

#include "../mesa/Variable.h"

static variable::Values values;

// mesa::variable
// mesa::variable dump

int returnDict(Tcl_Interp* interp) {
        values.set();
        auto dict = Tcl_NewDictObj();

        {
            uint64_t value = (uint64_t)values.PID[1] << 32 | values.PID[2] << 16 | values.PID[3];
            put(interp, dict, "PID", std_sprintf("0x%lX", value));
        }
        put(interp, dict, "WP", std_sprintf("0x%04X", values.WP));
        put(interp, dict, "WDC", std_sprintf("%d", values.WDC));
        put(interp, dict, "PTC", std_sprintf("0x%04X", values.PTC));
        put(interp, dict, "XTS", std_sprintf("%d", values.XTS));
        put(interp, dict, "PSB", std_sprintf("%d", values.PSB));
        put(interp, dict, "MDS", std_sprintf("0x%04X", values.MDS >> 16));
        put(interp, dict, "LF", std_sprintf("0x%04X", values.LF));
        put(interp, dict, "GF", std_sprintf("0x%08X", values.GF));
        put(interp, dict, "CB", std_sprintf("0x%08X", values.CB));
        put(interp, dict, "GFI", std_sprintf("0x%04X", values.GFI));
        put(interp, dict, "PC", std_sprintf("0x%04X", values.PC));
        put(interp, dict, "SP", std_sprintf("%d", values.SP));
        put(interp, dict, "savedPC", std_sprintf("0x%04X", values.savedPC));
        put(interp, dict, "savedSP", std_sprintf("%d", values.savedSP));
        {
            auto value = Tcl_NewListObj(0, 0);
            for(int i = 0; i < StackDepth; i++) {
                std::string string = std_sprintf("0x%04X", values.stack[i]);
                auto ret = Tcl_ListObjAppendElement(interp, value, Tcl_NewStringObj(string.c_str(), string.length()));
                if (ret != TCL_OK) ERROR()
            }
            put(interp, dict, "stack", value);
        }
        put(interp, dict, "breakByte", std_sprintf("0x%02X", values.breakByte));
        put(interp, dict, "running", values.running ? "1" : "0");
        put(interp, dict, "lastOpcode", opcode::lastOpcodeName());

        Tcl_SetObjResult(interp, dict);

        return TCL_OK;
}

int dump(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    auto subCommand = tcl::toString(objv[1]);

    if (subCommand == "dump") {
        variable::dump();
        return TCL_OK;
    }

    return invalidCommand(cdata, interp, objc, objv);
}

int MesaVariable(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    if (objc == 1) return returnDict(interp);
    if (objc == 2) return dump(cdata, interp, objc, objv);

    return invalidCommand(cdata, interp, objc, objv);
}
