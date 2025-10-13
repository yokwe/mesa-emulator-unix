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
// MesaGuam_config.cpp
//

#include <map>
#include <string>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "../mesa/guam.h"
#include "../mesa/guam_config.h"

#include "tclMesa.h"


#define FIELD_MAP_ENTRY(name) { #name, &config.name }
std::map<std::string, std::string*> stringMap = {
    FIELD_MAP_ENTRY(diskFilePath),
    FIELD_MAP_ENTRY(germFilePath),
    FIELD_MAP_ENTRY(bootFilePath),
    FIELD_MAP_ENTRY(floppyFilePath),
    FIELD_MAP_ENTRY(networkInterface),
    FIELD_MAP_ENTRY(bootSwitch),
    FIELD_MAP_ENTRY(bootDevice),
    FIELD_MAP_ENTRY(displayType),
};
std::map<std::string, int*> intMap = {
    FIELD_MAP_ENTRY(displayWidth),
    FIELD_MAP_ENTRY(displayHeight),
    FIELD_MAP_ENTRY(vmBits),
    FIELD_MAP_ENTRY(rmBits),
};


// mesa::guam config
// 0          1
// mesa::guam config name
// 0          1      2
// mesa::guam config load name
// 0          1      2    3
int MesaGuam_config(ClientData cdata, Tcl_Interp* interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;

    tcl::Interp interp(interp_);

    if (objc == 2) {
        // mesa::guam config
        // 0          1
        std::string result;
        for(auto i = stringMap.cbegin(); i != stringMap.cend(); i++) {
            auto string = std_sprintf("%-16s  \"%s\"\n", i->first, *i->second);
            result += string;
        }
        for(auto i = intMap.cbegin(); i != intMap.cend(); i++) {
            auto string = std_sprintf("%-13s  %4d\n", i->first, *i->second);
            result += string;
        }
        interp.result(result);
        return TCL_OK;
    }
    if (objc == 3) {
        // mesa::guam config name
        // 0          1      2
        std::string subject = tcl::toString(objv[2]);
        if (intMap.contains(subject)) {
            if (objc == 3) {
                int* p = intMap.at(subject);
                interp.result(*p);
                return TCL_OK;
            }
        }
        if (stringMap.contains(subject)) {
            if (objc == 3) {
                std::string* p = stringMap.at(subject);
                interp.result(*p);
                return TCL_OK;
            }
        }
    }
    if (objc == 4) {
        // mesa::guam config load GVWin
        // 0          1      2    3
        std::string subject = Tcl_GetString(objv[2]);
        if (subject == "load") {
            std::string entryName = Tcl_GetString(objv[3]);
            auto guamConfig = guam_config::getInstance();
            if (!guamConfig.containsEntry(entryName)) {
                auto string = std_sprintf("entry \"%s\" is not in setting", entryName);
                interp.result(string);
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
    }

    return TCL_ERROR;
}