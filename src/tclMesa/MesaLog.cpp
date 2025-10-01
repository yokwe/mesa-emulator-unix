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
// MasaLog.cpp
//

#include <tcl.h>
#include <tclDecls.h>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

int MesaLog(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    Tcl_Obj* result = 0;

    if (objc < 3) {
        result = Tcl_ObjPrintf("Unexpected objc is less than 3  objc = %d", objc);
        logger.error(Tcl_GetString(result));
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }
    // mesa::log info format args...
    // 0         1    2      3
    std::string level  = Tcl_GetString(objv[1]);
    const char* format = Tcl_GetString(objv[2]);

    result = Tcl_Format(interp, format, objc - 3, objv + 3);
    if (result == NULL) return TCL_ERROR;
	Tcl_SetObjResult(interp, result);

    if (level == "debug") {
        logger.debug(Tcl_GetString(result));
    } else if (level == "info") {
        logger.info(Tcl_GetString(result));
    } else if (level == "warn") {
        logger.warn(Tcl_GetString(result));
    } else if (level == "error") {
        logger.error(Tcl_GetString(result));
    } else if (level == "fatal") {
        logger.fatal(Tcl_GetString(result));
    } else {
        result = Tcl_ObjPrintf("Unexpected level \"%s\"", level.c_str());
        logger.error(Tcl_GetString(result));
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }

	return TCL_OK;
}
