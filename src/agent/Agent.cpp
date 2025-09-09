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
// Agent.cpp
//


#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/memory.h"

#include "Agent.h"

void Agent::Enable() {
	all.at(index) = this;
	fcbAddress = AllocFCB(index, fcbSize);
    logger.info("Agent %2d %-10s  fcb = %04X  %2d", index, name, fcbAddress, fcbSize);
	Initialize();
}

LONG_POINTER Agent::AllocFCB(int index, int size) {
    if (size == 0) ERROR();

	// setup ioRegionPtr if needed
    if (ioRegionPtr == 0) {
        ioRegionPtr = (GuamInputOutput::IORegionType*)memory::peek(ioRegion);
        memset(ioRegionPtr, 0, sizeof(GuamInputOutput::IORegionType));
        ioRegion += SIZE(GuamInputOutput::IORegionType);
    }
    // update fcbptrs
    ioRegionPtr->fcbptrs[index] = ioRegion;
    
    CARD32 ret = ioRegion;
    ioRegion += size + (size & 1);
    return ret;
}
