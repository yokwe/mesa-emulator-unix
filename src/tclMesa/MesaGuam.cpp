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
#include "../mesa/Setting.h"

#include "../opcode/opcode.h"

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
    FIELD_MAP_ENTRY(displayType),
};
std::map<std::string, int*> intMap = {
    FIELD_MAP_ENTRY(displayWidth),
    FIELD_MAP_ENTRY(displayHeight),
    FIELD_MAP_ENTRY(vmBits),
    FIELD_MAP_ENTRY(rmBits),
};

static std::string        imageName;
static Tk_PhotoHandle     photoHandle = 0;
static Tk_PhotoImageBlock imageBlock = {0, 0, 0, 0, 0, {0}};

void initialize(Tk_PhotoImageBlock& imageBlock, int width, int height) {
    if (imageBlock.pixelPtr) {
        ckfree(imageBlock.pixelPtr); // FIXME is this good?
    }
    imageBlock.width     = width;
    imageBlock.height    = height;
    imageBlock.pixelSize = 4; // red + gree + blue + alpha
    imageBlock.offset[0] = 0; // red
    imageBlock.offset[1] = 1; // green
    imageBlock.offset[2] = 2; // blue
    imageBlock.offset[3] = 3; // alpah
    // use 32 bit memory access
    int lineWidth = multipleOf(width, 32);
    imageBlock.pitch     = lineWidth * imageBlock.pixelSize;
    int totalByte = imageBlock.pitch * imageBlock.height;
    imageBlock.pixelPtr  = (unsigned char*)ckalloc(totalByte); // FIXEME is this good?
    // fill with white and full opaque
    memset(imageBlock.pixelPtr, 0xFF, totalByte);
}
int put(Tcl_Interp* interp, Tk_PhotoImageBlock& imageBlock) {
    return Tk_PhotoPutBlock(interp, photoHandle, &imageBlock, 0, 0, imageBlock.width, imageBlock.height, TK_PHOTO_COMPOSITE_SET);
}
void fill(Tk_PhotoImageBlock& imageBlock, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t* lineStart = imageBlock.pixelPtr;
    for(int line = 0; line < imageBlock.height; line++) {
        auto p = lineStart;
        for(int bit = 0; bit < imageBlock.width; bit++) {
            p[0] = r;
            p[1] = g;
            p[2] = b;
            p += imageBlock.pixelSize;
        }
        lineStart += imageBlock.pitch;
    }
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
            std::string entryName = Tcl_GetString(objv[2]);
            auto setting = Setting::getInstance();
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
        processor_thread::stopAtMP(8000);

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
    if (subCommand == "display") {
        if (objc == 5) {
            // mesa::guam display imageName width height
            // 0          1       2         3     4
            imageName = tcl::toString(objv[2]);
            auto width = toInt(interp, objv[3], status);
            if (status != TCL_OK) return status;
            auto height = toInt(interp, objv[4], status);
            if (status != TCL_OK) return status;

            initialize(imageBlock, width, height);
            fill(imageBlock, 255, 255, 255);

            photoHandle = Tk_FindPhoto(interp, imageName.c_str());
            put(interp, imageBlock);
            return TCL_OK;
        }
    }
    if (subCommand == "test") {
        if (objc == 6) {
            std::string subject = tcl::toString(objv[2]);
            // mesa::guam test display r g b
            // 0          1    2       3 4 5
            if (subject == "display") {
                int status;
                auto r = toInt(interp, objv[3], status);
                if (status != TCL_OK) return status;
                auto g = toInt(interp, objv[4], status);
                if (status != TCL_OK) return status;
                auto b = toInt(interp, objv[5], status);
                if (status != TCL_OK) return status;
                fill(imageBlock, r, g, b);
                put(interp, imageBlock);
                return TCL_OK;
            }
        }
    }

    if (subCommand == "keyPress" && objc == 4) {
        // mesa::guam keyPress keySymNumber keySymString
        // 0          1        2            3
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
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