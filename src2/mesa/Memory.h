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

#include "../util/Util.h"

#include "../mesa/Constant.h"
#include "../mesa/Type.h"

namespace mesa {

class Memory {
public:
	 Memory(): vpSize(0), rpSize(0), realMemory(0), memoryFlags(0) {}

	 void initialize(int vmBits, int rmBits, CARD16 ioRegionPage);

	 CARD16* getAddressk(CARD32 va);
	 CARD16* fetch(CARD32 va);
	 CARD16* store(CARD32 va);

	 int isVacant(CARD32 va) {
		 return memoryFlags[va / PageSize].vacant;
	 }

	 static inline int isSamePage(CARD32 vaA, CARD32 vaB) {
	 	return (vaA / PageSize) == (vaB / PageSize);
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
	 	void setVacant() {
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

	 CARD16 *realMemory;  // rpSize * PageSize
	 Flags  *memoryFlags; // vpSize
};

}
