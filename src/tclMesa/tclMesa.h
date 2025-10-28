/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/


//
// tclMesa.h
//

#pragma once

#include <tcl.h>

#include "../mesa/guam.h"

int MesaBoot     (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaConfig   (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaDisplay  (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaEvent    (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaLog      (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaMemory   (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaPerf     (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaTime     (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaTrace    (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int MesaVariable (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);

void refreshDisplay();

extern guam::Config config;

// LinkVAr
void PerfLinkVar(Tcl_Interp* interp);

inline void MesaLinkVar(Tcl_Interp *interp) {
    PerfLinkVar(interp);
}

// utility method
int invalidCommand  (ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);

void put(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, Tcl_Obj* value);
void put(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, const std::string& value);

void putINT16(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int16_t value);
void putUINT16(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint16_t value);
void putINT32(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int32_t value);
void putUINT32(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint32_t value);
void putINT64(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, int64_t value);
void putUINT64(Tcl_Interp *interp, Tcl_Obj* dict, const char* key, uint64_t value);

