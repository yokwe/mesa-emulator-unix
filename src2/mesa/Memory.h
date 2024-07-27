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


#pragma once

#include "../mesa/Type.h"

namespace mesa {

class Memory {
public:
	 static CARD32 PAGE_SIZE = 256;

	 Memory(int vmBits, int rmBits, CARD16 ioRegionPage);

	 CARD16* getAddressk(CARD32 va) {
		auto vp = va / PAGE_SIZE;
		auto of = va % PAGE_SIZE;

		auto flag = memoryFlags[vp];
		if (flag.vacant) ERROR();

		return realMemory + flag.offset() + of;
	 }

	 int isVacant(CARD32 va) {
		 return memoryFlag[va / PAGE_SIZE].vacant;
	 }

	 CARD16* fetch(CARD32 va) {
		auto vp = va / PAGE_SIZE;
		auto of = va % PAGE_SIZE;

		Flags flag = memoryFlags[vp];
		if (flag.vacant) PageFault(va);
		if (!flag.fetch) {
			flag.fetch = 1;
			memoryFlags[vp] = flag;
		}

		return realMemory + flag.offset() + of;
	 }
	 CARD16* store(CARD32 va) {
		const CARD32 vp = va / PAGE_SIZE;
		const CARD32 of = va % PAGE_SIZE;

		Flags flags = memoryFlags[vp];
		if (flags.vacant) PageFault(va);
		if (flags.protect) WriteProtectFault(va)
		if (!flags.store) {
			flag.store = 1;
			memoryFlags[vp] = flags;
		}

		return realMemory + flag.offset() + of;
	 }

	 static inline int isSamePage(CARD32 ptrA, CARD32 ptrB) {
	 	return (ptrA / PAGE_SIZE) == (ptrB / PAGE_SIZE);
	 }

private:
	 // 3.1.1 Virtual Memory Mapping
	 //MapFlags: TYPE = MACHINE DEPENDENT RECORD (
	 //  reserved (0:0..12) : UNSPEClFIED[0..17777B],
	 //  protected (0:13..13) : BOOLEAN,
	 //  dirty (0:14..14): BOOLEAN,
	 //  referenced (0:15..15): BOOLEAN];
	 union Flags {
	 	CARD32 u0;
	 	struct {
	 		CARD32 fetch      :  1;
	 		CARD32 store      :  1;
	 		CARD32 protect    :  1;
	 		CARD32 vacant     :  1;
	 		CARD32 reserved   :  4;
	 		//
	 		CARD32 realOffset : 16;
	 		//
	 		CARD32 reserved2  :  8;
	 	 };
	 	CARD32 offset() {
	 		return u0 & 0x00FFFF00;
	 	}
	 	void clear() {
	 		u0 = 0;
	 	}
	 	void vacant() {
	 		clear();
	 		vacant = 1;
	 	}
	 	int isReferenced() {
	 		return fetch | store;
	 	}
	 	int isDirty() {
	 		return store;
	 	}
	  };

	 int vpSize; // number of virtual page
	 int rpSize; // number of real page

	 CARD16[] realMemory;  // rpSize * PAGE_SIZE
	 Flags[]  memoryFlags; // vpSize
};

}
