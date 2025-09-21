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
// photo_image.h
//

#pragma once

#include <string>

#include <tcl.h>
#include <tk.h>


class PhotoImage {
    Tcl_Interp*        interp;
    std::string        name;
    Tk_PhotoHandle     handle;
    int                width;
    int                height;
    Tk_PhotoImageBlock imageBlock;

    void initialize();

    void finalize();

    void checkImageSize();
public:
    PhotoImage() :
        interp(0), handle(0), width(0), height(0) {
    }
    ~PhotoImage() {
        finalize();
    }

    void initialize(Tcl_Interp* interp, const std::string& name);

    bool isInitialzied() {
        return interp;
    }

    void putBlock();

    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void fill(uint8_t r, uint8_t g, uint8_t b) {
        fill(r, g, b, 0xFf);
    }
    void full(uint8_t rgb) {
        fill(rgb, rgb, rgb, 0xFF);
    }

    void copyMesaDisplay();
    void copyMesaDisplayMonochrome();

};
