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
// MasaMemory.cpp
//

#include <tcl.h>
#include <tclDecls.h>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "../mesa/memory.h"

static int memoryConfig(Tcl_Interp* interp) {
    auto dict = Tcl_NewDictObj();

    auto config = memory::getConfig();

    putUINT32(interp, dict, "rpSIze", config.rpSize);
    putUINT32(interp, dict, "vpSize", config.vpSize);

    {
        auto display = Tcl_NewDictObj();

        putINT32(interp, display, "pageSize", config.display.pageSize);
        putINT32(interp, display, "vp", config.display.vp);
        putINT32(interp, display, "rp", config.display.rp);
        put(interp, dict, "display", display);
    }

    Tcl_SetObjResult(interp, dict);
    return TCL_OK;
}
static int memoryMap(Tcl_Interp* interp, int address) {
    auto dict = Tcl_NewDictObj();

    const CARD32 vp = (CARD32)address / PageSize;
    memory::Map map= memory::ReadMap(vp);
    MapFlags mf = map.mf;
    CARD16 rp = map.rp;
    std::string string;
    if (mf.isVacant()) {
        string += " Vacant";
    } else {
        if (mf.dirty) string += " Dirty";
        if (mf.protect) string += " Protect";
        if (mf.referenced) string += " Referenced";
    }
    put(interp, dict, "mf", string.substr(1));
    putUINT16(interp, dict, "rp", rp);

    Tcl_SetObjResult(interp, dict);
    return TCL_OK;
}
static int memoryRead(Tcl_Interp* interp_, int address) {
    tcl::Interp interp(interp_);
    if (memory::isVacant(address)) {
        interp.result(-1);
    } else {
        interp.result(*memory::peek(address));
    }
    return TCL_OK;
}
static int memoryVacant(Tcl_Interp* interp_, int address) {
    auto intValue = memory::isVacant(address) ? 1 : 0;
    tcl::Interp interp(interp_);
    interp.result(intValue);
    return TCL_OK;
}


// mesa::memory config
// mesa::memory map    address
// mesa::memory vacant address
// mesa::memroy read   address
// mesa::memroy write  address value

int MesaMemory(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    if (objc == 2) {
        auto subCommand = tcl::toString(objv[1]);
        if (subCommand == "config") return memoryConfig(interp);
    }
    if (objc == 3) {
        auto subCommand = tcl::toString(objv[1]);
        int status;
        auto address = tcl::toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;

        if (subCommand == "map") return memoryMap(interp, address);
        if (subCommand == "read") return memoryRead(interp, address);
        if (subCommand == "vacant") return memoryVacant(interp, address);
    }

    return invalidCommand(cdata, interp, objc, objv);
}

