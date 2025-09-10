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
// Agent.h
//

#pragma once

#include "../mesa/MesaBasic.h"
#include "../mesa/Pilot.h"

class Agent {
public:
	const int   index;
	const char* name;
	const int   fcbSize;
	CARD32      fcbAddress;

	Agent(GuamInputOutput::AgentDeviceIndex index_, char const *name_, int fcbSize_) :
		index((int)index_), name(name_), fcbSize(fcbSize_), fcbAddress(0) {}
	virtual ~Agent() {}

	void Enable();

	virtual void Initialize() = 0;
	virtual void Call()       = 0;
};

namespace agent {
	// ioRegionLast: CARDINAL = 255;
	// ioRegionAfterEnd: CARDINAL = ioRegionLast + 1;
	static const CARD16 ioRegionLast = 255;
	static const CARD16 ioRegionAfterEnd = ioRegionLast + 1;

	// rp-vp  0x60-0x00, 0x61-0x01, ... 0xff-0x9f
	//   ioRegionPage = 0xff - 0x9f = 0x60
	//   ioRegionPageCount = 0x100 - 0x60 = 0xa0
	// ioRegionPage = ioRegionLast - PageMap.GetState[ioRegionLast].real;
	// ioRegionPtr = Environment.LongPointerFromPage[ioRegionPage];
	// ioRegionPageCount = ioRegionAfterEnd - Inline.LowHalf[ioRegionPage];
	static const CARD32 ioRegionPage = 0x80; // TODO What is a optimal value for ioRegionPage?  ?GermOpsImpl::pageEndGermVM?

	void          initialize();
	LONG_POINTER  getIORegion();
	LONG_POINTER  AllocFCB(int index, int fcbSize);

	void   addAgent(Agent* agent);
	Agent* getAgent(int index);
	void   callAgent(int index);
}
