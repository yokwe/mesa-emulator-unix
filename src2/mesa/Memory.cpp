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
// Memory.h
//

#include <algorithm>


#include "Memory.h"

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

namespace mesa {

Memory memory;

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
void Memory::initialize(CARD32 vmBits, CARD32 rmBits, CARD16 ioRegionPage) {
	// sanity check
	if (vmBits < VMBITS_MIN) ERROR();
	if (VMBITS_MAX < vmBits) ERROR();
	if (rmBits < RMBITS_MIN) ERROR();
	if (MRBITS_MAX < rmBits) ERROR();
	if (vmBits < rmBits)     ERROR();
	if (255 < ioRegionPage)  ERROR();

	delete memoryArray;
	delete flagArray;
	delete pageArray;

	vpSize      = 1 << (vmBits - PageBits);
	rpSize      = 1 << std::min(rmBits - PageBits, MAX_REALMEMORY_PAGE_SIZE);

	auto memoryArraySize = rpSize * PageSize;

	memoryArray = new CARD16[memoryArraySize];
	flagArray   = new Flag[vpSize];
	pageArray   = new CARD16*[vpSize];
	mds         = 0;

//	logger.info("vmBits  %2d  %4X", vmBits, vpSize - 1);
//	logger.info("rmBits  %2d  %4X", rmBits, rpSize - 1);

	for(CARD32 i = 0; i < memoryArraySize; i++) memoryArray[i] = 0;
	for(CARD32 i = 0; i < vpSize; i++) flagArray->clear();
	for(CARD32 i = 0; i < vpSize; i++) pageArray[i] = 0;

	//const int VP_START = pageGerm + countGermVM;
	CARD32 rp = 0;
	// vp:[ioRegionPage .. 256) <=> rp:[0..256-ioRegionPage)
	for(CARD32 i = ioRegionPage; i < 256; i++) {
		flagArray[i].clear();
		pageArray[i] = memoryArray + PageSize * rp++;
	}
	// vp:[0..ioRegionPage) <=> rp: [256-ioRegionPage .. 256)
	for(CARD32 i = 0; i < ioRegionPage; i++) {
		flagArray[i].clear();
		pageArray[i] = memoryArray + PageSize * rp++;
	}
	// vp: [256 .. rpSize)
	for(CARD32 i = 256; i < rpSize; i++) {
		flagArray[i].clear();
		pageArray[i] = memoryArray + PageSize * rp++;
	}
	if (rp != rpSize) ERROR();
	// vp: [rpSize .. vpSize)
	for(CARD32 i = rpSize; i < vpSize; i++) {
		flagArray[i].setVacant();
		pageArray[i] = 0;
	}
}


// WriteMap: PROCEDURE [virtual: VirtualPageNumber, flags: MapFlags, real: RealPageNumber];
 void Memory::writeMap(CARD32 vp, MapFlags mapFlags, CARD32 rp) {
	 if (vpSize <= vp) ERROR();
	 CARD16* page;
	 Flag    flag;

	 if (Vacant(mapFlags)) {
		 if (rp != 0) ERROR();
		 page = 0;
		 flag.setVacant();
	 } else {
		 page = pageArray[rp << PageBits];
		 flag.clear();
		 if (mapFlags.dirty)      flag.store   = 1;
		 if (mapFlags.protect)    flag.protect = 1;
		 if (mapFlags.referenced) flag.fetch   = 1;
	 }
	 pageArray[vp] = page;
	 flagArray[vp] = flag;

	 // FIXME update pageArrayMDS
 }
 std::pair<MapFlags, CARD32> Memory::readMap(CARD32 vp) {
	 if (vpSize <= vp) ERROR();
	 MapFlags mapFlags;
	 CARD32   rp;

	 auto page = pageArray[vp];
	 auto flag = flagArray[vp];

	 if (page == 0) {
		 if (!flag.vacant) ERROR();

		 mapFlags.dirty      = 1;
		 mapFlags.protect    = 1;
		 mapFlags.referenced = 0;
		 rp = 0;
	 } else {
		 mapFlags.u = 0;
		 if (flag.protect)        mapFlags.protect    = 1;
		 if (flag.isDirty())      mapFlags.dirty      = 1;
		 if (flag.isReferenced()) mapFlags.referenced = 1;

		 CARD32 memoryOffset = page - memoryArray;
		 if (memoryOffset & PageMask) ERROR();

		 rp = memoryOffset >> PageBits;
	 }

	 return {mapFlags, rp};
 }

}
