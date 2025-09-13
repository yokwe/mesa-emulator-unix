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

#include <string>
#include <map>
#include <utility>
#include <thread>

#include <tcl.h>
#include <tclDecls.h>
#include <tk.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/guam.h"
#include "../mesa/processor.h"
#include "../mesa/memory.h"

#include "../opcode/opcode.h"

#include "../util/Setting.h"
#include "../util/GuiOp.h"
#include "../util/Perf.h"
#include "../util/tcl.h"

#include "mesa.h"


static guam::Config config;

#define FIELD_MAP_ENTRY(name) { #name, &config.name }
std::map<std::string, std::string*> stringMap = {
    FIELD_MAP_ENTRY(diskFilePath),
    FIELD_MAP_ENTRY(germFilePath),
    FIELD_MAP_ENTRY(bootFilePath),
    FIELD_MAP_ENTRY(floppyFilePath),
    FIELD_MAP_ENTRY(networkInterface),
    FIELD_MAP_ENTRY(bootSwitch),
    FIELD_MAP_ENTRY(bootDevice),
};
std::map<std::string, int*> intMap = {
    FIELD_MAP_ENTRY(displayWidth),
    FIELD_MAP_ENTRY(displayHeight),
    FIELD_MAP_ENTRY(vmBits),
    FIELD_MAP_ENTRY(rmBits),
};

int MesaGuam(ClientData cdata, Tcl_Interp *interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    tcl::Interp interp(interp_);

    std::string command = tcl::toString(objv[0]);
    // mesa::guam config diskFile XXX
    // 0          1      2        3
    //            command subjet  value
    std::string subCommand = tcl::toString(objv[1]);
    if (subCommand == "config") {
        if (objc == 2) {
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
        if (objc == 3 || objc == 4) {
            std::string subject = tcl::toString(objv[2]);
            if (intMap.contains(subject)) {
                if (objc == 3) {
                    int* p = intMap.at(subject);
                    interp.result(*p);
                    return TCL_OK;
                }
                if (objc == 4) {
                    int* p = intMap.at(subject);
                    int status;
                    auto value = toInt(interp, objv[3], status);
                    if (status == TCL_OK) *p = value;
                    return status;
                }
            }
            if (stringMap.contains(subject)) {
                if (objc == 3) {
                    std::string* p = stringMap.at(subject);
                    interp.result(*p);
                    return TCL_OK;
                }
                if (objc == 4) {
                    std::string* p = stringMap.at(subject);
                    auto value = tcl::toString(objv[3]);
                    *p = value;
                    return TCL_OK;
                }
            }
        }
    }
    if (subCommand == "setting") {
        auto setting = Setting::getInstance();
        if (objc == 2) {
            std::string string = "valid entry for setting are ";
            for(auto i = setting.entryList.cbegin(); i != setting.entryList.cend(); i++) {
                string.append(std_sprintf(" %s", i->name));
            }
            interp.result(string);
            return TCL_OK;
        }
        if (objc == 3) {
            std::string entryName = Tcl_GetString(objv[2]);
            if (!setting.containsEntry(entryName)) {
                auto string = std_sprintf("no entry \"%s\" in setting", entryName);
                interp.result(string);
                return TCL_ERROR;
            }
            auto entry   = setting.getEntry(entryName);

            config.diskFilePath     = entry.file.disk;
            config.germFilePath     = entry.file.germ;
            config.bootFilePath     = entry.file.boot;
            config.floppyFilePath   = entry.file.floppy;
            config.networkInterface = entry.network.interface;
            config.bootSwitch       = entry.boot.switch_;
            config.bootDevice       = entry.boot.device;
            config.displayWidth     = entry.display.width;
            config.displayHeight    = entry.display.height;
            config.vmBits           = entry.memory.vmbits;
            config.rmBits           = entry.memory.rmbits;
            return TCL_OK;
        }
    }
    if (subCommand == "time" && objc == 2) {
        long elapsedTime = guam::getElapsedTime();
        interp.result(elapsedTime);
        return TCL_OK;
    }
    if (subCommand == "run" && objc == 2) {
	    guam::setConfig(config);

	    GuiOp::setContext(new NullGuiOp);

	    // stop at MP 8000
        processor::stopAtMP( 915);
        processor::stopAtMP(8000);

        logger.info("thread start");
        auto thread = std::thread(guam::run);
        logger.info("thread joinning");
        thread.join();
        logger.info("thread joined");
        return TCL_OK;
    }
    if (subCommand == "stats" && objc == 2) {
        opcode::stats();
        PERF_LOG();
        memory::cache::stats();
        return TCL_OK;
    }

    {
        auto string = std_sprintf("invalid command name \"%s", command);
        for(int i = 1; i < objc; i++) {
            string.append(std_sprintf(" %s", Tcl_GetString(objv[i])));
        }
        string.append("\"");

        interp.result(string);
        return TCL_ERROR;
    }
}