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

#include <tcl.h>
#include <tclDecls.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "keymap.h"

#include "tclMesa.h"

constexpr const char* PACKAGE_NAME    = "Mesa";
constexpr const char* PACKAGE_VERSION = "1.0.";

extern "C" int DLLEXPORT Mesa_Init(Tcl_Interp *interp) {
    logger.info("Mesa_Init");
	/* changed this to check for an error - GPS */
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) == TCL_ERROR) {
        logger.error("Tcl_PkgProvide failed");
		return TCL_ERROR;
	}

	Tcl_CreateObjCommand(interp, "mesa::boot",     MesaBoot,      NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::config",   MesaConfig,    NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::display",  MesaDisplay,   NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::event",    MesaEvent,     NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::log",      MesaLog,       NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::memory",   MesaMemory,    NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::perf",     MesaPerf,      NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::trace",    MesaTrace,      NULL, NULL);
	Tcl_CreateObjCommand(interp, "mesa::variable", MesaVariable,  NULL, NULL);

	MesaLinkVar(interp);

	return TCL_OK;
}

//
// refresh display
//
static void refreshDisplayTimerStart();
static constexpr int REFRESH_INTERVAL = 100; // in milli seconds
static void refreshDisplayTimerProc(void *) {
	// refreshDisplay takes about 2 ms
	refreshDisplay();
	// create timer for next iteration
	refreshDisplayTimerStart();
 }
void refreshDisplayTimerStart() {
	Tcl_CreateTimerHandler(REFRESH_INTERVAL, refreshDisplayTimerProc, 0);
}

int AppInit(Tcl_Interp *interp) {
	if (Tcl_Init(interp) == TCL_ERROR) {
        logger.fatal("Tcl_Init failed");
		return TCL_ERROR;
    }

    Mesa_Init(interp);

	keymap::initialize();

    auto scriptFile = std::filesystem::path("data/mesa.tcl");
    if (std::filesystem::exists(scriptFile)) {
        logger.info("eval  mesa scrip  %s", scriptFile.c_str());
        auto script = readFile(scriptFile);
        Tcl_Eval(interp, script.c_str());
    }

	// create timer to refreash display
	refreshDisplayTimerStart();

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

void put(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, Tcl_Obj* value) {
    int ret = Tcl_DictObjPut(interp, dict, tcl::toStringObj(key), value);
    if (ret != TCL_OK) ERROR()
}
void put(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, const std::string& value) {
    put(interp, dict, key, tcl::toStringObj(value));
}

void putINT16(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int16_t value) {
    put(interp, dict, key, tcl::toIntObj(value));
}
void putUINT16(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint16_t value) {
    put(interp, dict, key, tcl::toIntObj(value));
}
void putINT32(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int32_t value) {
    put(interp, dict, key, tcl::toIntObj(value));
}
void putUINT32(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint32_t value) {
    put(interp, dict, key, tcl::toIntObj(value));
}
void putINT64(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int64_t value) {
    put(interp, dict, key, tcl::toWideIntObj(value));
}
void putUINT64(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint64_t value) {
    put(interp, dict, key, tcl::toWideIntObj(value));
}
