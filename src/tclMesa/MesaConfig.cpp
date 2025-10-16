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
// MasaPerf.cpp
//

#include <string>

#include <tcl.h>
#include <tclDecls.h>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "../mesa/guam_config.h"

guam::Config config;

#define PUT_STRING(name) put(interp, dict, #name, config.name);
#define PUT_INT(name) putINT32(interp, dict, #name, config.name);

int MesaConfig(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    // mesa::config
    // 0
    if (objc == 1) {
        auto dict = Tcl_NewDictObj();
        
        PUT_STRING(diskFilePath)
        PUT_STRING(germFilePath)
        PUT_STRING(bootFilePath)
        PUT_STRING(floppyFilePath)
        PUT_STRING(networkInterface)
        PUT_STRING(bootSwitch)
        PUT_STRING(bootDevice)
        PUT_STRING(displayType)

        PUT_INT(displayWidth)
        PUT_INT(displayHeight)
        PUT_INT(vmBits)
        PUT_INT(rmBits)

        Tcl_SetObjResult(interp, dict);
        return TCL_OK;
    }
    // mesa::config entryName
    // 0            1
    if (objc == 2) {
        std::string entryName = Tcl_GetString(objv[1]);
        auto guamConfig = guam_config::getInstance();
        if (!guamConfig.containsEntry(entryName)) {
            auto string = std_sprintf("entry \"%s\" is not in config", entryName);
            Tcl_SetObjResult(interp, Tcl_NewStringObj(string.c_str(), string.length()));
            return TCL_ERROR;
        }

        auto entry   = guamConfig.getEntry(entryName);
        config.diskFilePath     = entry.file.disk;
        config.germFilePath     = entry.file.germ;
        config.bootFilePath     = entry.file.boot;
        config.floppyFilePath   = entry.file.floppy;
        config.networkInterface = entry.network.interface;
        config.bootSwitch       = entry.boot.switch_;
        config.bootDevice       = entry.boot.device;
        config.displayType      = entry.display.type;
        config.displayWidth     = entry.display.width;
        config.displayHeight    = entry.display.height;
        config.vmBits           = entry.memory.vmbits;
        config.rmBits           = entry.memory.rmbits;
        return TCL_OK;
    }

    return invalidCommand(cdata, interp, objc, objv);
}
