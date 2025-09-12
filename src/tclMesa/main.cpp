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
// main.cpp
//

#include <filesystem>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include <tcl.h>

#include "mesa.h"

constexpr const char* PACKAGE_NAME    = "Mesa";
constexpr const char* PACKAGE_VERSION = "1.0.";

extern "C" int DLLEXPORT Mesa_Init(Tcl_Interp *interp) {
    logger.info("Guam_Init");
	/* changed this to check for an error - GPS */
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) == TCL_ERROR) {
        logger.error("Tcl_PkgProvide failed");
		return TCL_ERROR;
	}
	Tcl_CreateObjCommand(interp, "mesa::log", MesaLog, NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::guam", MesaGuam, NULL, NULL);
	return TCL_OK;
}

int AppInit(Tcl_Interp *interp) {
	if (Tcl_Init(interp) == TCL_ERROR) {
        logger.fatal("Tcl_Init failed");
		return TCL_ERROR;
    }

    Mesa_Init(interp);

    auto scriptFile = std::filesystem::path(BUILD_DIR) / "run" / "mesa.tcl";
    if (std::filesystem::exists(scriptFile)) {
        logger.info("eval  mesa scrip  %s", scriptFile.c_str());
        auto script = readFile(scriptFile);
        Tcl_Eval(interp, script.c_str());
    }

	return TCL_OK;
}

int main(int argc, char *argv[]) {
    logger.info("START");
    
	setSignalHandler(SIGINT);
	setSignalHandler(SIGTERM);
	setSignalHandler(SIGHUP);
	setSignalHandler(SIGSEGV);

    Tcl_FindExecutable(argv[0]);
	Tcl_Main(argc, argv, AppInit);

    // Tcl_Main call ::exit() and so don't reach here
    logger.info("STOP");
	return 0;
}
