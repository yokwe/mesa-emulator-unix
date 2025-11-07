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
// MesaBoot.cpp
//

#include <cstring>
#include <thread>

#include <tcl.h>
#include <tclDecls.h>
#include <tk.h>
#include <tkDecls.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/guam.h"
#include "../mesa/processor.h"
#include "../mesa/Variable.h"

#include "../util/GuiOp.h"

#include "tclMesa.h"
 

// mesa::boot
// 0
int MesaBoot(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    if (objc == 1) {
        guam::setConfig(config);

        GuiOp::setContext(new NullGuiOp);
        MP.addObserver(GuiOp::setMP);

        // stop at MP 8000
        processor::stopAtMP( 915);

        logger.info("guam thread start");
        auto thread = std::thread(guam::run);
        thread.detach();
        logger.info("guam thread detached");
        return TCL_OK;
    }

    return invalidCommand(cdata, interp, objc, objv);
}