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
// MasaLinkVar.cpp
//

#include <string>

#include <tcl.h>
#include <tclDecls.h>

#include "tclMesa.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/Perf.h"

void PerfLinkVar(Tcl_Interp *interp) {
    std::string namespacePerf = "mesa::perf";
    if (Tcl_FindNamespace(interp, namespacePerf.c_str(), 0, 0) == 0) {
        auto ret = Tcl_CreateNamespace(interp, namespacePerf.c_str(), 0, 0);
        if (ret == 0) ERROR()
    }
    {
        for(const auto& e: perf::all) {
            std::string name = "mesa::perf::";
            name += e.name;

            // create namespace if not existed
            {
                auto pos = name.find_last_of("::");
                auto nameSpace = name.substr(0, pos - 1);
                if (Tcl_FindNamespace(interp, nameSpace.c_str(), 0, 0) == 0) {
                    auto ret = Tcl_CreateNamespace(interp, nameSpace.c_str(), 0, 0);
                    if (ret == 0) ERROR()
                }
            }

            logger.info("create linkvar  %s", name);
            auto ret = Tcl_LinkVar(interp, name.c_str(), (char*)&e.value, TCL_LINK_WIDE_UINT | TCL_LINK_READ_ONLY);
            if (ret != TCL_OK) ERROR()
        }
    }
}
