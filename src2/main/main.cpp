/*******************************************************************************
 * Copyright (c) 2024, Yasuhiro Hasegawa
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

#include <string>

#include "../mesa/Constant.h"
#include "../mesa/Type.h"
#include "../mesa/Memory.h"
#include "../mesa/Pilot.h"

#include "../util/Util.h"
static const util::Logger logger(__FILE__);


namespace mesa {

/*
CARD32 readDblA(CARD32 va) {
	CARD16* p0 = Memory::fetch(va + 0);
//	CARD16* p1 = Memory::isSamePage(va + 0, va + 1) ? (p0 + 1) : memory.fetch(va + 1);
	CARD16* p1 = ((va & 0xFF) == 0xFF) ? Memory::fetch(va + 1) : (p0 + 1) ;
	return (*p1 << WordSize) | *p0;
}
CARD32 readDblB(CARD32 va) {
	CARD16* p0 = Memory::fetch(va + 0);
	CARD16* p1 = Memory::isSamePage(va + 0, va + 1) ? (p0 + 1) : Memory::fetch(va + 1);
//	CARD16* p1 = ((va & 0xFF) == 0xFF) ? fetch(va + 1) : (p0 + 1) ;
	return (*p1 << WordSize) | *p0;
}
*/

}

int main(int /*argc*/, char** /*argv*/) {
	logger.info("START");

	util::setSignalHandler();

	DEBUG_TRACE();

	logger.debug("%3d", 3);

	util::logBackTrace();

	logger.info("AAA");
	util::Logger::pushLevel();
	logger.info("BBB");
	util::Logger::popLevel();
	logger.info("CCC");

	mesa::CARD32 vmBits = 24;
	mesa::CARD32 rmBits = 20;
	mesa::memory.initialize(vmBits, rmBits);

	logger.info("STOP");
}