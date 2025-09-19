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
// display.h
//

#pragma once

#include "MesaBasic.h"

namespace display {

class DisplayContext {
public:
	CARD16 type;
	CARD16 width;
	CARD16 height;

	CARD32 wordsPerLine;
	CARD32 words;
	CARD32 pagesForBitmap;

    CARD32 displayMemoryAddress;

    DisplayContext() :
        type(0), width(0), height(0), wordsPerLine(0), words(0), pagesForBitmap(0), displayMemoryAddress(0) {}
	DisplayContext(CARD16 type_, CARD16 width_, CARD16 height_, CARD32 wordsPerLine_, CARD32 words_, CARD32 pagesForBitmap_) :
		type(type_), width(width_), height(height_), wordsPerLine(wordsPerLine_), words(words_), pagesForBitmap(pagesForBitmap_), displayMemoryAddress(0) {}

	static DisplayContext getInstance(CARD16 type, CARD16 width, CARD16 height);
};

}