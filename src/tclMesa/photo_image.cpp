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

#include <bit>

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


class MesaMonoSource {
public:
    const int MASK_INIT = 0x8000;

    int height;
    int width;
    int wordsPerLine;

    CARD16* line;
    CARD16* p;
    int x;
    int y;
    int mask;
    int word;

    MesaMonoSource(CARD16* bitmap, const display::Config& config) {
        height = config.height;
        width = config.width;
        wordsPerLine = config.wordsPerLine;

        line = bitmap;
        p  = line;
        x = 0;
        y = 0;
        mask = MASK_INIT;
        word = std::byteswap(*p);
    };

    int test() {
        return word & mask;
    }
    int next() {
//        logger.info("mesa  next  %4d  %4d", x, y);
        if ((x + 1) < width) {
            x++;
            if (x & 0x0F) {
                mask >>= 1;
            } else {
                // advance word
                mask = MASK_INIT;
                p++;
                word = std::byteswap(*p);
            }
        } else {
            if ((y + 1) < height) {
                // advance line
                y++;
                line += wordsPerLine;
                p = line;
                x = 0;
                mask = MASK_INIT;
                word = std::byteswap(*p);
            } else {
                // reach to end
                return 0;
            }
        }
        return 1;
    }
};
class PhotoDest {
public:
    CARD8 *pixelPtr;	// Pointer to the first pixel.
    int width;			// Width of block, in pixels.
    int height;			// Height of block, in pixels.
    int pitch;			// Address difference between corresponding pixels in successive lines.
    int pixelSize;		// Address difference between successive pixels in the same line.
    int offsetR;        // Address differences between the red, green, blue and alpha components of the pixel and the pixel as a whole.
    int offsetG;
    int offsetB;
    int offsetA;

    CARD8* line;
    CARD8* p;
    int x;
    int y;

    PhotoDest(Tk_PhotoImageBlock& imageBlock) {
        pixelPtr  = imageBlock.pixelPtr;
        width     = imageBlock.width;
        height    = imageBlock.height;
        pitch     = imageBlock.pitch;
        pixelSize = imageBlock.pixelSize;
        offsetR   = imageBlock.offset[0];
        offsetG   = imageBlock.offset[1];
        offsetB   = imageBlock.offset[2];
        offsetA   = imageBlock.offset[3];

        line = pixelPtr;
        p    = line;
        x    = 0;
        y    = 0;
    };
    void set(CARD8 r, CARD8 g, CARD8 b, CARD8 a) {
        p[offsetR] = r;
        p[offsetG] = g;
        p[offsetB] = b;
        p[offsetA] = a;
    }
    void set(CARD8 rgb, CARD8 a = 0xFF) {
        set(rgb, rgb, rgb, a);
    }
    int next() {
//        logger.info("photo next  %4d  %4d", x, y);
        x++;
        if (x < width) {
            // advance
            p += pixelSize;
        } else {
            y++;
            if (y < height) {
                // advance line
                line += pitch;
                p = line;
                x = 0;
            } else {
                return 0;
            }
        }
        return 1;
    }
};


void PhotoImage::copyMesaDisplay() {
    const auto memoryConfig = memory::getConfig();
    const auto displayConfig = display::getConfig();

    // sanity check
    checkImageSize();
    if (displayConfig.width != width || displayConfig.height != height) ERROR();
    if (displayConfig.type != DisplayIOFaceGuam::T_monochrome) ERROR();
    if (memoryConfig.display.bitmap == 0) ERROR();

    MesaMonoSource source{memoryConfig.display.bitmap, displayConfig};
    PhotoDest dest{imageBlock};
    int totalDot = width * height;
    for(int i = 0; i < totalDot; i++) {
        if (i) {
            source.next();
            dest.next();
        }
       CARD8 rgb = source.test() ? 0x00 : 0xFF;
       (void)rgb;
       dest.set(rgb);
    }
}