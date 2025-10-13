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
// tcl.h
//

#pragma once

#include <string>

#include <tcl.h>
#include <tclDecls.h>

namespace tcl {

inline std::string toString(Tcl_Obj* obj) {
    std::string ret(Tcl_GetString(obj));
    return ret;
}
inline int toInt(Tcl_Interp* interp, Tcl_Obj* obj, int& status) {
    int ret;
    status = Tcl_GetIntFromObj(interp, obj, &ret);
    return ret;
}

inline Tcl_Obj* toStringObj(const std::string& string) {
    const char* bytes = string.c_str();
    int length = string.length();
    return Tcl_NewStringObj(bytes, length);
}
inline Tcl_Obj* toIntObj(int value) {
    return Tcl_NewIntObj(value);
}
inline Tcl_Obj* toWideIntObj(Tcl_WideInt value) {
    return Tcl_NewWideIntObj(value);
}

class Interp {
    Tcl_Interp* interp;
public:
    Interp() = delete;
    Interp(Tcl_Interp* interp_) : interp(interp_) {}

    operator Tcl_Interp*() {
        return interp;
    }

    void result(const std::string& string) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(string.c_str(), string.length()));
    }
    void result(int value) {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(value));
    }
    void result(long value) {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(value));
    }
    std::string result() {
        std::string ret(Tcl_GetStringResult(interp));
        return ret;
    }
    bool hasResult() {
        auto result = Tcl_GetObjResult(interp);
        return result->length || result->typePtr;
    }
};

class Obj {
    Interp&  interp;
    Tcl_Obj* obj;

    void logFunctionFailed(const char* function);
public:
    Obj(Interp& interp_) : interp(interp_), obj(Tcl_NewObj()) {}
    Obj(Interp& interp_, Tcl_Obj* obj_) : interp(interp_), obj(obj_) {
        Tcl_IncrRefCount(obj); // TODO Is this correct?
    }

    ~Obj() {
        Tcl_DecrRefCount(obj); // TODO Is this correct?
    }

    operator Tcl_Obj*() {
        return obj;
    }

    // std::string
    Obj& operator=(const std::string& that) {
        Tcl_SetStringObj(obj, that.c_str(), that.length());
        return *this;
    }
    Obj& operator+=(const std::string& that) {
        Tcl_AppendToObj(obj, that.c_str(), that.length());
        return *this;
    }
    operator std::string() {
        return toString(obj);
    }
    // int
    Obj& operator=(int that) {
        Tcl_SetIntObj(obj, that);
        return *this;
    }
    int toInt(int& status) {
        int ret = 0;
        status = Tcl_GetIntFromObj(interp, obj, &ret);
        if (status == TCL_OK) return ret;
        logFunctionFailed("Tcl_GetIntFromObj");
        return -1;
    }
    // long
    Obj& operator=(long that) {
        Tcl_SetLongObj(obj, that);
        return *this;
    }
    long toLong(int& status) {
        long ret = 0;
        status = Tcl_GetLongFromObj(interp, obj, &ret);
        if (status == TCL_OK) return ret;
        logFunctionFailed("Tcl_GetLongFromObj");
        return -1;
    }
    // double
    Obj& operator=(double that) {
        Tcl_SetDoubleObj(obj, that);
        return *this;
    }
    double toDouble(int& status) {
        double ret = 0;
        status = Tcl_GetDoubleFromObj(interp, obj, &ret);
        if (status == TCL_OK) return ret;
        logFunctionFailed("Tcl_GetDoubleFromObj");
        return -1;
    }
    // obj
    Obj& operator+=(const Obj& that) {
        Tcl_AppendObjToObj(obj, that.obj);
        return *this;
    }
};

}