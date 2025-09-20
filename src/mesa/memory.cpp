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
// memory.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Pilot.h"

#include "memory.h"

namespace memory {

Config config;
const Config& getConfig() {
	return config;
}


constexpr int MAX_REALMEMORY_PAGE_SIZE = RealMemoryImplGuam::largestArraySize * WordSize;
constexpr int VMBITS_MIN               = 20;
constexpr int VMBITS_MAX               = 25;

Map*    maps      = 0;
CARD16* pages     = 0;
Page**  realPage  = 0;

static void initializeVariables() {
	maps     = 0;
	pages    = 0;
	realPage = 0;
	config.clear();
}

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
void initialize(int vmBits, int rmBits, CARD16 ioRegionPage) {
	if (config.vpSize) ERROR();
	if (vmBits < VMBITS_MIN) ERROR();
	if (VMBITS_MAX < vmBits) ERROR();
	if (vmBits < rmBits) ERROR();

	initializeVariables();

	config.vpSize = 1 << (vmBits - Environment::logWordsPerPage);
	config.rpSize = 1 << (rmBits - Environment::logWordsPerPage);

	if (MAX_REALMEMORY_PAGE_SIZE < config.rpSize) config.rpSize = MAX_REALMEMORY_PAGE_SIZE;

//	logger.info("vmBist = %d  vpSize = %6d  %4X", vmBits, vpSize, vpSize);
//	logger.info("rmBist = %d  rpSize = %6d  %4X", rmBits, rpSize, rpSize);

	// allocate pages.
	pages = new CARD16[config.rpSize * PageSize];
	// initialize for valgrind
	memset(pages, 0, sizeof(CARD16) * config.rpSize * PageSize);

	// allocate realPage and assign their values.
	realPage = new Page*[config.rpSize];
	for(CARD32 i = 0; i < config.rpSize; i++) {
		realPage[i] = (Page *)(pages + i * PageSize);
	}

	MapFlags vacant = {6};
	MapFlags clear = {0};

	maps = new Map[config.vpSize];
	//const int VP_START = pageGerm + countGermVM;
	CARD32 rp = 0;
	// vp:[ioRegionPage .. 256) <=> rp:[0..256-ioRegionPage)
	for(int i = ioRegionPage; i < 256; i++) {
		maps[i].mf = clear;
		maps[i].rp = rp++;
	}
	// vp:[0..ioRegionPage) <=> rp: [256-ioRegionPage .. 256)
	for(int i = 0; i < ioRegionPage; i++) {
		maps[i].mf = clear;
		maps[i].rp = rp++;
	}
	// vp: [256 .. rpSize)
	for(CARD32 i = 256; i < config.rpSize; i++) {
		maps[i].mf = clear;
		maps[i].rp = rp++;
	}
	if (rp != config.rpSize) ERROR();
	// vp: [rpSize .. vpSize)
	for(CARD32 i = config.rpSize; i < config.vpSize; i++) {
		maps[i].mf = vacant;
		maps[i].rp = 0;
	}

	// initialize related class
	cache::initialize();
}

void finalize() {
	delete[] maps;
	delete[] realPage;
	delete[] pages;

	initializeVariables();
}

void reserveDisplayPage(int displayPageSize) {
	config.display.pageSize = displayPageSize;

	// grab allocated real memory for display
	const CARD32 vp = config.rpSize - config.display.pageSize;
	config.display.rp = maps[vp].rp;

	// make [rpSize - displayPageSize..rpSize) vacant
	MapFlags vacant = {6};
	Map map;
	map.mf = vacant;
	map.rp = 0;
	for(int i = 0; i < displayPageSize; i++) {
		WriteMap(vp + i, map);
	}

	logger.info("%s  %6X+%X", __FUNCTION__, config.rpSize, config.display.pageSize);
}

void mapDisplay(CARD32 vp, CARD32 rp, CARD16 pageCount, CARD16 pageCountInEachBlock) {
	logger.info("%s  %6X+%X  %6X %X %X", __FUNCTION__, vp, pageCount, rp, pageCount, pageCountInEachBlock);
	if (rp != (CARD32)config.display.rp) {
		logger.fatal("rp                = %d", rp);
		logger.fatal("config.display.rp = %d", config.display.rp);
		ERROR();
	}
	if (pageCount != config.display.pageSize) {
		logger.fatal("pageCount       = %d", pageCount);
		logger.fatal("config.display.pageSize = %d", config.display.pageSize);
		ERROR();
	}
	config.display.vp = vp;

	// for(CARD32 i = 0; i < pageCount; i++) {
	// 	Map map = maps[vp + i];
	// 	logger.info("map  %8X  %8X  %8X  %2X", vp + i, rp + i, map.rp, map.mf.u);
	// }

	// IMPORTANT Need to assign rp and clear MapFLags
	// make [vp..vp + pageCount) clear
	MapFlags clear = {0};
	for(CARD32 i = 0; i < pageCount; i++) {
		Map map;
		map.mf = clear;
		map.rp = rp + i;
		WriteMap(vp + i, map);
	}
}

bool isDisplayMapped() {
	return 	config.isDisplayMapped();
}


int isDisplayPage(int vp) {
	if (config.display.pageSize == 0) ERROR();
	return config.isDisplayPage(vp);
}

CARD16* FetchPage(CARD32 vp) {
	PERF_COUNT(memory, FetchPage)
	if (config.vpSize <= vp) ERROR();
	Map *p = maps + vp;
	Map map = *p;
	if (map.mf.isVacant()) PageFault(vp * PageSize);
	if (map.mf.referenced == 0) {
		map.mf.referenced = 1;
		p->mf = map.mf;
	}
	Page* page = realPage[map.rp];
	if (page == 0) ERROR();
	//
	return page->word;
}
CARD16* StorePage(CARD32 vp) {
	PERF_COUNT(memory, StorePage)
	if (config.vpSize <= vp) ERROR();
	Map *p = maps + vp;
	Map map = *p;
	if (map.mf.isVacant()) PageFault(vp * PageSize);
	if (map.mf.isProtect()) WriteProtectFault(vp * PageSize);
	if (map.mf.referenced == 0 || map.mf.dirty == 0) {
		map.mf.referenced = 1;
		map.mf.dirty      = 1;
		p->mf = map.mf;
	}
	Page* page = realPage[map.rp];
	if (page == 0) ERROR();
	//
	return page->word ;
}
CARD16* peek(CARD32 va) {
	PERF_COUNT(memory, GetAddress)
	const CARD32 vp = va / PageSize;
	const CARD32 of = va % PageSize;
	if (config.vpSize <= vp) ERROR()
	Map map = maps[vp];
	if (map.mf.isVacant()) {
		logger.fatal("%s  va = %6X  vp = %4X", __FUNCTION__, va, vp);
		logger.fatal("%s  mf = %4X  rp = %4X", __FUNCTION__, map.mf.u + 0, map.rp + 0);
		ERROR();
	}
	Page* page = realPage[map.rp];
	if (page == 0) ERROR();
	//
	return page->word + of;
}

void setReferencedFlag(CARD32 vp) {
	if (config.vpSize <= vp) ERROR();
	Map *p = maps + vp;
	p->mf.referenced = 1;
}
void setReferencedDirtyFlag(CARD32 vp) {
	if (config.vpSize <= vp) ERROR();
	Map *p = maps + vp;
	p->mf.referenced = 1;
	p->mf.dirty      = 1;
}

Map ReadMap(CARD32 vp) {
	if (config.vpSize <= vp) ERROR();
	Map map = maps[vp];
	if (map.mf.isVacant()) map.rp = 0;
	return map;
}
void WriteMap(CARD32 vp, Map map) {
	if (config.vpSize <= vp) ERROR();
	if (config.rpSize <= map.rp) ERROR();

	if (map.mf.isVacant()) map.rp = 0;

	maps[vp] = map;
	PERF_COUNT(memory, WriteMap)
	cache::invalidate(vp);
}

namespace cache {
	uint64_t  missConflict = 0;
	uint64_t  missEmpty    = 0;
	uint64_t  hit          = 0;
	Entry     entry[N_ENTRY];

	void initialize() {
		for(auto i = 0U; i < N_ENTRY; i++) {
			entry[i].clear();
		}
		hit          = 0;
		missEmpty    = 0;
		missConflict = 0;
	}
	void invalidate(CARD32 vp_) {
		Entry *p = getEntry(vp_);
		if (p->vpno == vp_) {
			// void entry of vp_
			p->flag = 0;
			p->page = 0;
		}
	}
	void stats() {
		int used = 0;
		for(CARD32 i = 0; i < N_ENTRY; i++) {
			if (entry[i].vpno) used++;
		}

		if (PERF_ENABLE) {
			uint64_t total = (missEmpty + missConflict) + hit;
			auto totalString = formatWithCommas(total);
			auto missEmptyString = formatWithCommas(missEmpty);
			auto missConflictString = formatWithCommas(missConflict);

			logger.info("PageCache %5d / %5d  %s  %6.2f%%   miss empty %s  conflict %s",
				used, N_ENTRY, totalString, ((double)hit / total) * 100.0, missEmptyString, missConflictString);
		} else {
			logger.info("PageCache %5d / %5d", used, N_ENTRY);
		}
	}

	void fetchSetup(Entry *p, CARD32 vp) {
		if (PERF_ENABLE) {
			if (p->vpno) missConflict++;
			else missEmpty++;
		}
		// Overwrite content of entry
		p->page      = memory::FetchPage(vp);
		// NO PAGE FAULT AFTER HERE
		p->vpno      = vp;
		p->flagFetch = 1;
		p->flagStore = 0;
	}
	void fetchMaintainFlag(Entry *p, CARD32 vp) {
		memory::setReferencedFlag(vp);
		p->flagFetch = 1;
		// FIXME Should p->flagStore set to 0?
		//   There can be a chance that fetch after store in same page.
		// p->flagStore = 0;
	}
	void storeSetup(Entry *p, CARD32 vp) {
		if (PERF_ENABLE) {
			if (p->vpno) missConflict++;
			else missEmpty++;
		}
		// Overwrite content of entry
		p->page      = memory::StorePage(vp);
		// NO PAGE FAULT AFTER HERE
		p->vpno      = vp;
		p->flagFetch = 1;
		p->flagStore = 1;
	}
	void storeMaintainFlag(Entry *p, CARD32 vp) {
		memory::setReferencedDirtyFlag(vp);
		p->flagFetch = 1;
		p->flagStore = 1;
	}
}

}
