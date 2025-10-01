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
// display.cpp
//

#include "MesaBasic.h"
#include "Pilot.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "display.h"

namespace display {

static Config config;
const Config& getConfig() {
	return config;
}
void initialize(CARD16 type, CARD16 width, CARD16 height) {
	// from Apilot/15.3/Faces/Private/UserTerminalHeadGuam.mesa
	//      CalculateDisplayPages: PROC RETURNS [Environment.PageCount]
	const CARD32 wordsPerDWord = 2;
	const CARD32 bitsPerDWord = Environment::bitsPerWord * wordsPerDWord;
	CARD32 wordsPerLine;
	CARD32 wordSize;
	switch(type) {
	case DisplayIOFaceGuam::T_monochrome:
		wordsPerLine = ((width + bitsPerDWord - 1) / bitsPerDWord) * wordsPerDWord;
		wordSize = wordsPerLine * height;
		break;
	case DisplayIOFaceGuam::T_fourBitPlaneColor:
		wordsPerLine = ((width + bitsPerDWord - 1) / bitsPerDWord) * wordsPerDWord;
		wordSize = wordsPerLine * height * 4;
		break;
	case DisplayIOFaceGuam::T_byteColor:
		wordsPerLine = ((width + Environment::bytesPerPage - 1) / Environment::bytesPerPage) * Environment::wordsPerPage;
		wordSize = wordsPerLine * height;
		break;
	default:
		logger.error("Unexpected type  %d", type);
		ERROR();
		wordsPerLine = 0;
		wordSize = 0;
		break;
	}
	CARD32 pageSize = (wordSize + Environment::wordsPerPage - 1) / Environment::wordsPerPage;

	config.type         = type;
	config.width        = width;
	config.height       = height;
	config.wordsPerLine = wordsPerLine;
	config.wordSize     = wordSize;
	config.pageSize     = pageSize;

	logger.info("type         %6d", type);
	logger.info("width        %6d", width);
	logger.info("height       %6d", height);
	logger.info("wordsPerLine %6d", wordsPerLine);
	logger.info("wordSize     %6d", wordSize);
	logger.info("pageSize     %6d", pageSize);
}

}