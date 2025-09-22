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
// photo_image.cpp
//

#include <tcl.h>
#include <tclDecls.h>
#include <tk.h>
#include <tkDecls.h>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/memory.h"
#include "../mesa/display.h"
#include "../mesa/Pilot.h"

#include "photo_image.h"

void PhotoImage::initialize(Tcl_Interp* interp_, const std::string& name_) {
    if (interp_ == 0) ERROR();
    if (interp) ERROR(); // dont' expect called more than once

    interp = interp_;
    name   = name_;

    handle = Tk_FindPhoto(interp, name.c_str());
    if (handle == NULL) ERROR();

    Tk_PhotoGetSize(handle, &width, &height);

    imageBlock.width     = width;
    imageBlock.height    = height;
    imageBlock.pixelSize = 4; // red + gree + blue + alpha
    imageBlock.offset[0] = 0; // red
    imageBlock.offset[1] = 1; // green
    imageBlock.offset[2] = 2; // blue
    imageBlock.offset[3] = 3; // alpah
    imageBlock.pitch     = width * imageBlock.pixelSize;
    int totalByte = imageBlock.pitch * imageBlock.height;
    // allocate memory
    imageBlock.pixelPtr  = (unsigned char*)ckalloc(totalByte);
    // fill with white and full opaque
    memset(imageBlock.pixelPtr, 0xFF, totalByte);
}

void PhotoImage::finalize() {
    // free allocated memory
    ckfree(imageBlock.pixelPtr);
}

void PhotoImage::checkImageSize() {
    int width, height;
    Tk_PhotoGetSize(handle, &width, &height);
    if (width != this->width || height != this->height) ERROR();
}

void PhotoImage::putBlock() {
    // sanity check
    checkImageSize();

    int ret = Tk_PhotoPutBlock(interp, handle, &imageBlock, 0, 0, imageBlock.width, imageBlock.height, TK_PHOTO_COMPOSITE_SET);
    if (ret != TCL_OK) ERROR();
}

void PhotoImage::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint8_t* lineStart = imageBlock.pixelPtr;
    for(int line = 0; line < imageBlock.height; line++) {
        auto p = lineStart;
        for(int bit = 0; bit < imageBlock.width; bit++) {
            p[0] = r;
            p[1] = g;
            p[2] = b;
            p[3] = a;
            p += imageBlock.pixelSize;
        }
        lineStart += imageBlock.pitch;
    }
}


#define PROCESS_BIT_32_A(bitPos)  { b = (word & (0x80000000 >> bitPos)) ? 0x00 : 0xFF; *p++ = b; *p++ = b; *p++ = b; p++; }
#define PROCESS_BIT_32_B(bitPos)  { if (x == width) break; x++; b = (word & (0x80000000 >> bitPos)) ? 0x00 : 0xFF; *p++ = b; *p++ = b; *p++ = b; p++; }
void PhotoImage::copyMesaDisplayMonochrome32() {
    const auto memoryConfig = memory::getConfig();
    const auto displayConfig = display::getConfig();

    int wordsPerLine = displayConfig.wordsPerLine;
    int pixelSize    = imageBlock.pixelSize;
    int pitch        = imageBlock.pitch;

    if (pixelSize != 4) ERROR();

    uint16_t* bitmapLine = memoryConfig.display.bitmap;
    uint8_t*  pixelLine  = imageBlock.pixelPtr;

    int a = width / 32;

    for(int y = 0;;) {
        // start line
        uint32_t* s = (uint32_t*)bitmapLine;
        uint8_t*  p = pixelLine;

        int word = *s;
        int x = 0;
        int b;
        for(int i = 0; i < a; i++) {
            // process one word
            PROCESS_BIT_32_A(24)
            PROCESS_BIT_32_A(25)
            PROCESS_BIT_32_A(26)
            PROCESS_BIT_32_A(27)
            PROCESS_BIT_32_A(28)
            PROCESS_BIT_32_A(29)
            PROCESS_BIT_32_A(30)
            PROCESS_BIT_32_A(31)

            PROCESS_BIT_32_A(16)
            PROCESS_BIT_32_A(17)
            PROCESS_BIT_32_A(18)
            PROCESS_BIT_32_A(19)
            PROCESS_BIT_32_A(20)
            PROCESS_BIT_32_A(21)
            PROCESS_BIT_32_A(22)
            PROCESS_BIT_32_A(23)

            PROCESS_BIT_32_A(8)
            PROCESS_BIT_32_A(9)
            PROCESS_BIT_32_A(10)
            PROCESS_BIT_32_A(11)
            PROCESS_BIT_32_A(12)
            PROCESS_BIT_32_A(13)
            PROCESS_BIT_32_A(14)
            PROCESS_BIT_32_A(15)

            PROCESS_BIT_32_A(0)
            PROCESS_BIT_32_A(1)
            PROCESS_BIT_32_A(2)
            PROCESS_BIT_32_A(3)
            PROCESS_BIT_32_A(4)
            PROCESS_BIT_32_A(5)
            PROCESS_BIT_32_A(6)
            PROCESS_BIT_32_A(7)
            // prepare for next word
            x += 32;
            word = *s++;
        }

        for(;;) {
            // process one word
            PROCESS_BIT_32_B(24)
            PROCESS_BIT_32_B(25)
            PROCESS_BIT_32_B(26)
            PROCESS_BIT_32_B(27)
            PROCESS_BIT_32_B(28)
            PROCESS_BIT_32_B(29)
            PROCESS_BIT_32_B(30)
            PROCESS_BIT_32_B(31)

            PROCESS_BIT_32_B(16)
            PROCESS_BIT_32_B(17)
            PROCESS_BIT_32_B(18)
            PROCESS_BIT_32_B(19)
            PROCESS_BIT_32_B(20)
            PROCESS_BIT_32_B(21)
            PROCESS_BIT_32_B(22)
            PROCESS_BIT_32_B(23)

            PROCESS_BIT_32_B(8)
            PROCESS_BIT_32_B(9)
            PROCESS_BIT_32_B(10)
            PROCESS_BIT_32_B(11)
            PROCESS_BIT_32_B(12)
            PROCESS_BIT_32_B(13)
            PROCESS_BIT_32_B(14)
            PROCESS_BIT_32_B(15)
            
            PROCESS_BIT_32_B(0)
            PROCESS_BIT_32_B(1)
            PROCESS_BIT_32_B(2)
            PROCESS_BIT_32_B(3)
            PROCESS_BIT_32_B(4)
            PROCESS_BIT_32_B(5)
            PROCESS_BIT_32_B(6)
            PROCESS_BIT_32_B(7)
            break;
        }
        y++;
        if (y == height) break;
        // prepare for next line
        bitmapLine += wordsPerLine;
        pixelLine  += pitch;
    }
}

#define PROCESS_BIT_16_A(bitPos)  { b = (word & (0x8000 >> bitPos)) ? 0x00 : 0xFF; *p++ = b; *p++ = b; *p++ = b; p++; }
#define PROCESS_BIT_16_B(bitPos)  { if (x == width) break; x++; b = (word & (0x8000 >> bitPos)) ? 0x00 : 0xFF; *p++ = b; *p++ = b; *p++ = b; p++; }
void PhotoImage::copyMesaDisplayMonochrome16() {
    const auto memoryConfig = memory::getConfig();
    const auto displayConfig = display::getConfig();

    int wordsPerLine = displayConfig.wordsPerLine;
    int pixelSize    = imageBlock.pixelSize;
    int pitch        = imageBlock.pitch;

    if (pixelSize != 4) ERROR();

    uint16_t* bitmapLine = memoryConfig.display.bitmap;
    uint8_t*  pixelLine  = imageBlock.pixelPtr;

    int a = width / 16;

    for(int y = 0;;) {
        // start line
        uint16_t* s = bitmapLine;
        uint8_t*  p = pixelLine;

        int word = *s;
        int x = 0;
        int b;
        for(int i = 0; i < a; i++) {
            // process one word
            PROCESS_BIT_16_A(8)
            PROCESS_BIT_16_A(9)
            PROCESS_BIT_16_A(10)
            PROCESS_BIT_16_A(11)
            PROCESS_BIT_16_A(12)
            PROCESS_BIT_16_A(13)
            PROCESS_BIT_16_A(14)
            PROCESS_BIT_16_A(15)
            PROCESS_BIT_16_A(0)
            PROCESS_BIT_16_A(1)
            PROCESS_BIT_16_A(2)
            PROCESS_BIT_16_A(3)
            PROCESS_BIT_16_A(4)
            PROCESS_BIT_16_A(5)
            PROCESS_BIT_16_A(6)
            PROCESS_BIT_16_A(7)
            // prepare for next word
            x += 16;
            word = *s++;
        }

        for(;;) {
            // process one word
            PROCESS_BIT_16_B(8)
            PROCESS_BIT_16_B(9)
            PROCESS_BIT_16_B(10)
            PROCESS_BIT_16_B(11)
            PROCESS_BIT_16_B(12)
            PROCESS_BIT_16_B(13)
            PROCESS_BIT_16_B(14)
            PROCESS_BIT_16_B(15)
            PROCESS_BIT_16_B(0)
            PROCESS_BIT_16_B(1)
            PROCESS_BIT_16_B(2)
            PROCESS_BIT_16_B(3)
            PROCESS_BIT_16_B(4)
            PROCESS_BIT_16_B(5)
            PROCESS_BIT_16_B(6)
            PROCESS_BIT_16_B(7)
            break;
        }
        y++;
        if (y == height) break;
        // prepare for next line
        bitmapLine += wordsPerLine;
        pixelLine  += pitch;
    }
}

void PhotoImage::copyMesaDisplay() {
    const auto memoryConfig = memory::getConfig();
    const auto displayConfig = display::getConfig();

    // sanity check
    checkImageSize();
    if (displayConfig.width != width || displayConfig.height != height) ERROR();
    if (memoryConfig.display.bitmap == 0) ERROR();

    if (displayConfig.type == DisplayIOFaceGuam::T_monochrome) {
        copyMesaDisplayMonochrome32();
    } else {
        ERROR()
    }
}
