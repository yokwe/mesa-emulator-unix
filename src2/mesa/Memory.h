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

#include <utility>

#include "../util/Util.h"

#include "../mesa/Constant.h"
#include "../mesa/Type.h"
#include "../mesa/Function.h"

namespace mesa {

// 3.1.1 Virtual Memory Mapping
//MapFlags: TYPE = MACHINE DEPENDENT RECORD (
//  reserved (0:0..12) : UNSPEClFIED[0..17777B],
//  protected (0:13..13) : BOOLEAN,
//  dirty (0:14..14): BOOLEAN,
//  referenced (0:15..15): BOOLEAN];
union MapFlags {
	CARD16 u0;
	struct {
		CARD16 referenced :  1;
		CARD16 dirty      :  1;
		CARD16 protect    :  1;
		CARD16 reserved   : 13;
	};

	void clear() {
		u0 = 0;
	}
	void setVacant() {
		referenced = 0;
		dirty      = 1;
		protect    = 1;
		reserved   = 0;
	}
	int isVacant() {
		return !referenced && dirty && protect;
	}
};


int vpSize; // number of virtual page
int rpSize; // number of real page

class Memory {
public:
	static const CARD32 MAX_REALMEMORY_PAGE_SIZE = 4086; // RealMemoryImplGuam::largestArraySize * WordSize;
	static const CARD16 DEFAULT_IO_REGION_PAGE   =  128; // See Agent::ioRegionPage

	static const CARD32 VMBITS_MIN = 20;
	static const CARD32 VMBITS_MAX = 25;

	static const CARD32 RMBITS_MIN = 20;
	static const CARD32 MRBITS_MAX = 24;


	Memory(): vpSize(0), rpSize(0), memoryArray(0), flagArray(0), pageArray(0), mds(0) {
		 //
	}

	void initialize(CARD32 vmBits, CARD32 rmBits, CARD16 ioRegionPage);

	void initialize(CARD32 vmBits, CARD32 rmBits) {
		initialize(vmBits, rmBits, DEFAULT_IO_REGION_PAGE);
	}

	CARD32 getPage(CARD32 va) {
		return va / PageSize;
	}
	CARD32 getOffset(CARD32 va) {
		return va % PageSize;
	}

	CARD16* getAddress(CARD32 va) {
		auto page = pageArray[getPage(va)];
		if (page == 0) PageFault(va);

		return page + getOffset(va);
	}
	CARD16* fetch(CARD32 va) {
		auto vp   = getPage(va);
		auto page = pageArray[vp];
		auto flag = flagArray[vp];
		// check flag
		if (flag.vacant) PageFault(va);
		if (!flag.fetch) {
			// maintain flag
			flag.fetch = 1;
			 flagArray[vp] = flag;
		}

		return page + getOffset(va);
	}
	CARD16* store(CARD32 va) {
		// maintain flags
		auto vp   = getPage(va);
		auto page = pageArray[vp];
		auto flag = flagArray[vp];

		if (flag.vacant)  PageFault(va);
		if (flag.protect) WriteProtectFault(va);
		if (!flag.store) {
			// maintain flag
			flag.store = 1;
			flagArray[vp] = flag;
		}

		return page + getOffset(va);
	}

	int isVacant(CARD32 va) {
		return flagArray[getPage(va)].vacant;
	}

	//
	// map flag
	//
	void writeMap(CARD32 vp, MapFlags flag, CARD32 rp);
	std::pair<MapFlags, CARD32> readMap(CARD32 vp);

	//
	//  MDS
	//
	void setMDS(CARD32 newValue) {
		mds = newValue;
	}
	CARD32 getMDS() {
		return mds;
	}
	CARD32 lengthenPointer(CARD16 ptr) {
		return mds + ptr;
	}
	CARD16* getAddressMDS(CARD16 ptr) {
		return getAddress(lengthenPointer(ptr));
	}

private:
	union Flag {
		CARD8 u0;
		struct {
			CARD8 fetch      :  1;
			CARD8 store      :  1;
			CARD8 protect    :  1;
			CARD8 vacant     :  1;
		};
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

	CARD32 vpSize; // number of virtual page
	CARD32 rpSize; // number of real page

	CARD16  *memoryArray;  // rpSize * PageSize
	Flag    *flagArray;    // vpSize
	CARD16 **pageArray;    // vpSize;

	CARD32  mds;
};

extern Memory memory;


int isSamePage(CARD32 vaA, CARD32 vaB) {
	return (vaA / PageSize) == (vaB / PageSize);
}

void WriteMap(CARD32 vp, MapFlags flag, CARD32 rp) {
	memory.writeMap(vp, flag, rp);
}
std::pair<MapFlags, CARD32> ReadMap(CARD32 vp) {
	return memory.readMap(vp);
}


//
// Access Memory
//
CARD16* Fetch(CARD32 va) {
	return memory.fetch(va);
}
CARD16* Store(CARD32 va) {
	return memory.store(va);
}
CARD32 ReadDbl(CARD32 va) {
	CARD16* p0 = memory.fetch(va + 0);
	CARD16* p1 = (va & PageMask) == (PageSize - 1) ? memory.fetch(va + 1) : p0 + 1;
	return (*p1 << WordSize) | *p0;
}


//
// MDS
//
void setMDS(CARD32 va) {
	memory.setMDS(va);
}
CARD32 getMDS() {
	return memory.getMDS();
}
CARD32 lengthenPointer(CARD16 ptr) {
	return memory.lengthenPointer(ptr);
}

//
// Access MDS memory
// No need to maintain flags
//
CARD16* FetchMDS(CARD16 ptr) {
	return memory.getAddressMDS(ptr);
}
CARD16* StoreMDS(CARD16 ptr) {
	return memory.getAddressMDS(ptr);
}
CARD32 ReadDblMDS(CARD16 ptr) {
	CARD16* p0 = memory.getAddressMDS(ptr + 0);
	CARD16* p1 = (ptr & PageMask) == (PageSize - 1) ? memory.getAddressMDS(ptr + 1) : p0 + 1;
	return (*p1 << WordSize) | *p0;
}


}
