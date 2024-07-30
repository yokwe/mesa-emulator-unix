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

//
// Memory.cpp
//

#include <utility>

#include "Type.h"
#include "Constant.h"
#include "Variable.h"
#include "Function.h"

#include "Memory.h"

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

namespace mesa {

static const CARD32 MAX_REALMEMORY_PAGE_SIZE = RealMemoryImplGuam::largestArraySize;

static const CARD32 VMBITS_MIN = 20;
static const CARD32 VMBITS_MAX = 25;

static const CARD32 RMBITS_MIN = 20;
static const CARD32 MRBITS_MAX = 24;


union Flag {
	CARD32 u0;
	struct {
		CARD32 offset     : 24;
		//
		CARD32 fetch      :  1;
		CARD32 store      :  1;
		CARD32 protect    :  1;
		CARD32 vacant     :  1;
	};
	void clear() {
		u0 = 0;
	}
	void clear(CARD32 newValue) {
		clear();
		offset = newValue;
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

static CARD32 vpSize = 0; // number of virtual page
static CARD32 rpSize = 0; // number of real page

static CARD16* memoryArray = 0;  // rpSize * PageSize
static Flag*   flagArray   = 0;    // vpSize

void MemoryInit(CARD32 vmBits, CARD32 rmBits, CARD16 ioRegionPage) {
	// sanity check
	if (vmBits < VMBITS_MIN) ERROR();
	if (VMBITS_MAX < vmBits) ERROR();
	if (rmBits < RMBITS_MIN) ERROR();
	if (MRBITS_MAX < rmBits) ERROR();
	if (vmBits < rmBits)     ERROR();
	if (255 < ioRegionPage)  ERROR();

	delete memoryArray;
	delete flagArray;

	vpSize      = 1 << (vmBits - PageBits);
	rpSize      = 1 << (rmBits - PageBits);

	if (MAX_REALMEMORY_PAGE_SIZE < rpSize) rpSize = MAX_REALMEMORY_PAGE_SIZE;

	memoryArray = new CARD16[rpSize * PageSize];
	flagArray   = new Flag[vpSize];
	MDS         = 0;

	for(CARD32 i = 0; i < rpSize * PageSize; i++) memoryArray[i] = 0;
	for(CARD32 i = 0; i < vpSize; i++) flagArray[i].clear();

	//  From APilot/15.3/Pilot/Private/GermOpsImpl.mesa
	//	The BOOTING ACTION defined by the Principles of Operation should include:
	//	   1. Put real memory behind any special processor pages (I/O pages);
	//        then put all remaining usable real memory behind other virtual memory pages beginning at virtual page 0,
	//        and working upward sequentially (skipping any already-mapped special processor pages).
	//	   2. Read consecutive pages of the Germ into virtual memory beginning at
	//        page Boot.pageGerm + Boot.mdsiGerm*Environment.MaxPagesInMDS (i.e page 1).
	//        At present, the way the initial microcode figures out the device and device address of the Germ,
	//        and the number of pages which comprise the Germ, is processor-dependent.
	//	   3. Set Boot.pRequest = to e.g. [Boot.currentRequestBasicVersion, Boot.bootPhysicalVolume, locationOfPhysicalVolume].
	//	   4. Set WDC>0, NWW=0, MDS=Boot.mdsiGerm, STKP=0.
	//	   5. Xfer[dest: Boot.pInitialLink].

	//const int VP_START = pageGerm + countGermVM;
	CARD32 rp = 0;
	// vp:[ioRegionPage .. 256) <=> rp:[0..256-ioRegionPage)
	for(CARD32 i = ioRegionPage; i < 256; i++) {
		CARD32 offset = PageSize * rp++;
		flagArray[i].clear(offset);
	}
	// vp:[0..ioRegionPage) <=> rp: [256-ioRegionPage .. 256)
	for(CARD32 i = 0; i < ioRegionPage; i++) {
		CARD32 offset = PageSize * rp++;
		flagArray[i].clear(offset);
	}
	// vp: [256 .. rpSize)
	for(CARD32 i = 256; i < rpSize; i++) {
		CARD32 offset = PageSize * rp++;
		flagArray[i].clear(offset);
	}
	if (rp != rpSize) ERROR();
	// vp: [rpSize .. vpSize)
	for(CARD32 i = rpSize; i < vpSize; i++) {
		flagArray[i].setVacant();
	}
}

void WriteMap(CARD32 vp, MapFlags mapFlags, CARD32 rp) {
	 if (vpSize <= vp) ERROR();
	 Flag    flag;

	 if (Vacant(mapFlags)) {
		 if (rp != 0) ERROR();
		 flag.setVacant();
	 } else {
		 flag.clear();
		 if (mapFlags.dirty)      flag.store   = 1;
		 if (mapFlags.protect)    flag.protect = 1;
		 if (mapFlags.referenced) flag.fetch   = 1;
		 flag.offset = rp * PageSize;
	 }
	 flagArray[vp] = flag;
}
std::pair<MapFlags, CARD32> ReadMap(CARD32 vp) {
	 if (vpSize <= vp) ERROR();
	 MapFlags mapFlags;
	 CARD32   rp;

	 mapFlags.u = 0;
	 auto flag = flagArray[vp];

	 if (flag.vacant) {
		 mapFlags.dirty      = 1;
		 mapFlags.protect    = 1;
		 mapFlags.referenced = 0;
		 rp = 0;
	 } else {
		 if (flag.protect)        mapFlags.protect    = 1;
		 if (flag.isDirty())      mapFlags.dirty      = 1;
		 if (flag.isReferenced()) mapFlags.referenced = 1;

		 CARD32 offset = flag.offset;
		 if (offset & PageMask) ERROR();

		 rp = flag.offset / PageSize;
	 }

	 return {mapFlags, rp};
}

CARD16* GetAddress(CARD32 va) {
	auto flag = flagArray[va / PageSize];
	if (flag.vacant) PageFault(va);

	return memoryArray + flag.offset + (va % PageSize);
}
CARD16* Fetch(CARD32 va) {
	auto flag = flagArray[va / PageSize];
	if (flag.vacant) PageFault(va);

	if (!flag.fetch) {
		flag.fetch = 1;
		flagArray[va / PageSize] = flag;
	}

	return memoryArray + flag.offset + (va % PageSize);
}
CARD16* Store(CARD32 va) {
	auto flag = flagArray[va / PageSize];
	if (flag.vacant)  PageFault(va);
	if (flag.protect) WriteProtectFault(va);

	if (!flag.store) {
		flag.store = 1;
		flagArray[va / PageSize] = flag;
	}

	return memoryArray + flag.offset + (va % PageSize);
}

}
