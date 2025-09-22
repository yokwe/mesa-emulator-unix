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
// MesaGuam_display.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "../mesa/memory.h"

#include "photo_image.h"
#include "tclMesa.h"


static PhotoImage tkDisplay;

void refreshDisplay() {
    // if photo image is not created, return
    if (!tkDisplay.isInitialzied()) return;
    // if mesa display is not mapped, return
    if (memory::getConfig().display.bitmap == 0) return;
    tkDisplay.copyMesaDisplay();
    tkDisplay.updateImage();
}

// mesa::guam display refresh
// 0          1       2
// mesa::guam display set imageName
// 0          1       2   3
// mesa::guam display fill r g b
// 0          1       2    3 4 5

int MesaGuam_display(ClientData cdata, Tcl_Interp* interp_, int objc, Tcl_Obj *const objv[]) {
    (void)cdata;
    tcl::Interp interp(interp_);

    if (objc == 3) {
        // mesa::guam display imageName
        // mesa::guam display refresh
        // 0          1       2
        // register photo image for display
        auto name = tcl::toString(objv[2]);
        tkDisplay.initialize(interp, name);
        tkDisplay.full(0xFF); // 0xFF means WHITE
        tkDisplay.updateImage();
        return TCL_OK;
    }
    if (objc == 4) {
        // mesa::guam display set imageName
        // 0          1       2   3
        auto name = tcl::toString(objv[3]);
        if (name == "refresh") {
            if (memory::getConfig().display.bitmap) {
                refreshDisplay();
                return TCL_OK;
            } else {
                std::string result{"mesa display is not mapped"};
                logger.info(result);
                interp.result(result);
                return TCL_ERROR;
            }
        }
        tkDisplay.initialize(interp, name);
        tkDisplay.full(0xFF); // 0xFF means WHITE
        tkDisplay.updateImage();
        return TCL_OK;

    }
    if (objc == 6) {
        // mesa::guam display fill r g b
        // 0          1       2    3 4 5
        // fill desiplay with rgb values
        std::string subject = tcl::toString(objv[2]);
        if (subject == "fill") {
            int status;
            auto r = toInt(interp, objv[3], status);
            if (status != TCL_OK) return status;
            auto g = toInt(interp, objv[4], status);
            if (status != TCL_OK) return status;
            auto b = toInt(interp, objv[5], status);
            if (status != TCL_OK) return status;
            tkDisplay.fill(r, g, b);
            tkDisplay.updateImage();
            return TCL_OK;
        }
    }
    return TCL_ERROR;
}