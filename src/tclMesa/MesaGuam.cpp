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
// MasaGuam.cpp
//

#include <cstring>
#include <string>

#include <tcl.h>
#include <tclDecls.h>
#include <tk.h>
#include <tkDecls.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/guam.h"
#include "../mesa/memory.h"

#include "../opcode/opcode.h"

#include "../util/Perf.h"
#include "../util/tcl.h"

#include "tclMesa.h"


guam::Config config;

int MesaGuam(ClientData cdata, Tcl_Interp *interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    tcl::Interp interp(interp_);

    std::string command = tcl::toString(objv[0]);
    std::string subCommand;
    if (2 <= objc) subCommand.append(tcl::toString(objv[1]));

    // bind .mesa.display <Motion>        { mesa::guam motion %x %y }
    // bind .mesa.display <KeyPress>      { mesa::guam keyPress %N %K }
    // bind .mesa.display <KeyRelease>    { mesa::guam keyRelease %N %K }
    // bind .mesa.display <ButtonPress>   { mesa::guam buttonPress %b }
    // bind .mesa.display <ButtonRelease> { mesa::guam buttonRelease %b }
    if (subCommand == "motion" && objc == 4) {
        // mesa::guam motion x y
        // 0          1      2 3
        int status;
        auto x = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto y = toInt(interp, objv[3], status);
        if (status != TCL_OK) return status;
        guam::motion(x, y);
        return TCL_OK;
    }
    if (subCommand == "keyPress" && objc == 4) {
        // mesa::guam keyPress keySymNumber keySymString
        // 0          1        2            3
        int status;
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        guam::keyPress(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "keyRelease" && objc == 4) {
        // mesa::guam keyRelease keySymNumber keySymString
        // 0          1        2            3
        int status;
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        guam::keyRelease(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "buttonPress" && objc == 3) {
        // mesa::guam buttonPress buttonNumber
        // 0          1           2
        int status;
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        guam::buttonPress(buttonNumber);
        return TCL_OK;
    }
    if (subCommand == "buttonRelease" && objc == 3) {
        // mesa::guam buttonRelease buttonNumber
        // 0          1             2
        int status;
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        guam::buttonRelease(buttonNumber);
        return TCL_OK;
    }

    if (subCommand == "boot") {
        if (MesaGuam_boot(cdata, interp_, objc, objv) == TCL_OK) return TCL_OK;
    }
    if (subCommand == "config") {
        if (MesaGuam_config(cdata, interp_, objc, objv) == TCL_OK) return TCL_OK;
    }
    if (subCommand == "display") {
        if (MesaGuam_display(cdata, interp_, objc, objv) == TCL_OK) return TCL_OK;
    }
    if (subCommand == "stats" && objc == 2) {
        opcode::stats();
        PERF_LOG();
        memory::cache::stats();
        long elapsedTime = guam::getElapsedTime();
        logger.info("elapesd time  %llu", elapsedTime);
        return TCL_OK;
    }

    return invalidCommand(cdata, interp_, objc, objv);
}

int invalidCommand(ClientData cdata, Tcl_Interp *interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    tcl::Interp interp(interp_);
    if (!interp.hasResult()) {
        std::string commandString = tcl::toString(objv[0]);
        for(int i = 1; i < objc; i++) {
            commandString.append(" ");
            commandString.append(Tcl_GetString(objv[i]));
        }
        auto string = std_sprintf("invalid command name \"%s\"", commandString);
        interp.result(string);
    }
    return TCL_ERROR;
}
