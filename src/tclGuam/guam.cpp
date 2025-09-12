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
// guam.cpp
//

#include <tcl.h>

#include "guam.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

constexpr const char* PACKAGE_NAME    = "Guam";
constexpr const char* PACKAGE_VERSION = "1.0.";

static int Hello_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	(void)cdata;
	(void)objc;
	(void)objv;
	Tcl_SetObjResult(interp, Tcl_NewStringObj("Hello", -1));
	logger.info("hello");
	return TCL_OK;
}

static int Boot_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	(void)cdata;
	(void)objc;
	(void)objv;
	Tcl_SetObjResult(interp, Tcl_NewStringObj("Boot", -1));
	logger.info("boot");
	return TCL_OK;
}

extern "C" int DLLEXPORT Guam_Init(Tcl_Interp *interp) {
    logger.info("Guam_Init");
	/* changed this to check for an error - GPS */
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) == TCL_ERROR) {
        logger.error("Tcl_PkgProvide failed");
		return TCL_ERROR;
	}
	Tcl_CreateObjCommand(interp, "guam::hello", Hello_Cmd, NULL, NULL);
	Tcl_CreateObjCommand(interp, "guam::boot", Boot_Cmd, NULL, NULL);
	return TCL_OK;
}
