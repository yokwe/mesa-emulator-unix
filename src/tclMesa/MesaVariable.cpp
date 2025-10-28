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

#include "../opcode/opcode.h"

#include "../util/tcl.h"

#include "../mesa/Variable.h"

struct Values {
    CARD16 PID[4]; // Processor ID
    CARD16 MP;     // Maintenance Panel
    CARD16 WP;     // Wakeup pending register - 10.4.4.1
    CARD16 WDC;    // Wakeup disable counter - 10.4.4.3
    CARD16 PTC;    // Process timeout counter - 10.4.5
    CARD16 XTS;    // Xfer trap status - 9.5.5
    CARD16 PSB;    // PsbIndex - 10.1.1
    CARD32 MDS;    // Main Data Space
    CARD16 LF;     // POINTER TO LocalVariables
    CARD32 GF;     // LONG POINTER TO GlobalVarables
    CARD32 CB;     // LONG POINTER TO CodeSegment
    CARD16 GFI;
    CARD16 PC;
    CARD16 SP;
    CARD16 savedPC;
    CARD16 savedSP;
    CARD16 stack[StackDepth]; // Evaluation Stack - 3.3.2
    CARD8  breakByte;
    bool   running;

    void set() {
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
    }
};

static Values values;

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
    values.set();
    auto subCommand = tcl::toString(objv[1]);

    if (subCommand == "dump") {
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
        output.push_back(std::make_pair("lastOpcode", opcode::lastOpcodeName()));

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

        return TCL_OK;
    }

    return invalidCommand(cdata, interp, objc, objv);
}

int MesaVariable(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    if (objc == 1) return returnDict(interp);
    if (objc == 2) return dump(cdata, interp, objc, objv);

    return invalidCommand(cdata, interp, objc, objv);
}
