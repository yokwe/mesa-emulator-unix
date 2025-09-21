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

#include <cstring>
#include <string>
#include <map>
#include <utility>
#include <thread>

#include <tcl.h>
#include <tclDecls.h>
#include <tk.h>
#include <tkDecls.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/guam.h"
#include "../mesa/processor_thread.h"
#include "../mesa/memory.h"
#include "../mesa/setting.h"

#include "../opcode/opcode.h"

#include "../util/GuiOp.h"
#include "../util/Perf.h"
#include "../util/tcl.h"

#include "photo_image.h"

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
    FIELD_MAP_ENTRY(displayType),
};
std::map<std::string, int*> intMap = {
    FIELD_MAP_ENTRY(displayWidth),
    FIELD_MAP_ENTRY(displayHeight),
    FIELD_MAP_ENTRY(vmBits),
    FIELD_MAP_ENTRY(rmBits),
};

static PhotoImage tkDisplay;

static void guamDisplayMesa() {
    if (memory::getConfig().display.bitmap == 0) return;
    tkDisplay.copyMesaDisplay();
    tkDisplay.putBlock();
}

int MesaGuam(ClientData cdata, Tcl_Interp *interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    tcl::Interp interp(interp_);

    int status;
    std::string command = tcl::toString(objv[0]);
    // mesa::guam config diskFile XXX
    // 0          1      2        3
    //            subCommand
    //                   subject  value
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
       if (objc == 3) {
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
            std::string subject = tcl::toString(objv[2]);
            if (subject == "load") {
                auto setting = Setting::getInstance();
                std::string entryName = Tcl_GetString(objv[3]);
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
                config.displayType      = entry.display.type;
                config.displayWidth     = entry.display.width;
                config.displayHeight    = entry.display.height;
                config.vmBits           = entry.memory.vmbits;
                config.rmBits           = entry.memory.rmbits;
                return TCL_OK;
            }
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
        MP.addObserver(GuiOp::setMP);

	    // stop at MP 8000
        processor_thread::stopAtMP( 915);
//      processor_thread::stopAtMP(8000);

        logger.info("guam thread start");
        auto thread = std::thread(guam::run);
        thread.detach();
        logger.info("guam thread detached");
        return TCL_OK;
    }
    if (subCommand == "stats" && objc == 2) {
        opcode::stats();
        PERF_LOG();
        memory::cache::stats();
        return TCL_OK;
    }
    if (subCommand == "display") {
        if (objc == 2) {
            // mesa::guam display
            if (memory::getConfig().display.bitmap) {
                guamDisplayMesa();
                return TCL_OK;
            } else {
                std::string result{"mesa display is not mapped"};
                logger.info(result);
                interp.result(result);
                return TCL_ERROR;
            }
        }
        if (objc == 3) {
            // mesa::guam display imageName
            // mesa::guam display mesa
            // 0          1       2
            auto name = tcl::toString(objv[2]);
            tkDisplay.initialize(interp, name);
            tkDisplay.full(0xFF);
            tkDisplay.putBlock();
            return TCL_OK;
        }
        if (objc == 6) {
            // mesa::guam display set r g b
            // 0          1       2   3 4 5
            std::string subject = tcl::toString(objv[2]);
            if (subject == "set") {
                int status;
                auto r = toInt(interp, objv[3], status);
                if (status != TCL_OK) return status;
                auto g = toInt(interp, objv[4], status);
                if (status != TCL_OK) return status;
                auto b = toInt(interp, objv[5], status);
                if (status != TCL_OK) return status;
                tkDisplay.fill(r, g, b);
                tkDisplay.putBlock();
                return TCL_OK;
            }
        }
    }

// bind .mesa.display <KeyPress>      { keyPress %K }
// bind .mesa.display <KeyRelease>    { keyRelease %K }
// bind .mesa.display <ButtonPress>   { mouseButtonPress %b }
// bind .mesa.display <ButtonRelease> { mouseButtonRelease %b }
// bind .mesa.display <Motion>        { mouseMotion %x %y }
    if (subCommand == "keyPress" && objc == 4) {
        // mesa::guam keyPress keySymNumber keySymString
        // 0          1        2            3
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        if (keySymString == "Escape")  guamDisplayMesa(); // FIXME Make ESC key redraw display
        guam::keyPress(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "keyRelease" && objc == 4) {
        // mesa::guam keyRelease keySymNumber keySymString
        // 0          1        2            3
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        guam::keyRelease(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "buttonPress" && objc == 3) {
        // mesa::guam buttonPress buttonNumber
        // 0          1           2
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        guam::buttonPress(buttonNumber);
        return TCL_OK;
    }
    if (subCommand == "buttonRelease" && objc == 3) {
        // mesa::guam buttonRelease buttonNumber
        // 0          1             2
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        guam::buttonRelease(buttonNumber);
        return TCL_OK;
    }
    if (subCommand == "motion" && objc == 4) {
        // mesa::guam motion x y
        // 0          1      2 3
        auto x = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto y = toInt(interp, objv[3], status);
        if (status != TCL_OK) return status;
        guam::motion(x, y);
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