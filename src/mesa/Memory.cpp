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
// Memory.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Memory.h"


// class Memory
CARD32         Memory::vpSize              = 0;
CARD32         Memory::rpSize              = 0;
Memory::Map   *Memory::maps                = 0;
CARD16        *Memory::pages               = 0;
Memory::Page **Memory::realPage            = 0;
CARD32         Memory::displayPageSize     = 0;
CARD32         Memory::displayRealPage     = 0;
CARD32         Memory::displayVirtualPage  = 0;
CARD32         Memory::displayWidth        = 0;
CARD32         Memory::displayHeight       = 0;
CARD32         Memory::displayBytesPerLine = 0;


// Implementation Specific
static const int VMBITS_MIN = 20;
static const int VMBITS_MAX = 25;
void Memory::initialize(int vmBits, int rmBits, CARD16 ioRegionPage) {
	if (vpSize) ERROR();

	if (vmBits < VMBITS_MIN) ERROR();
	if (VMBITS_MAX < vmBits) ERROR();
	if (vmBits < rmBits) ERROR();

	vpSize = 1 << (vmBits - Environment::logWordsPerPage);
	rpSize = 1 << (rmBits - Environment::logWordsPerPage);
	displayRealPage = 0;
	displayPageSize = 0;

	if (MAX_REALMEMORY_PAGE_SIZE < rpSize) rpSize = MAX_REALMEMORY_PAGE_SIZE;

//	logger.info("vmBist = %d  vpSize = %6d  %4X", vmBits, vpSize, vpSize);
//	logger.info("rmBist = %d  rpSize = %6d  %4X", rmBits, rpSize, rpSize);

	// allocate pages.
	pages = new CARD16[rpSize * PageSize];
	// initialize for valgrind
	memset(pages, 0, sizeof(CARD16) * rpSize * PageSize);

	// allocate realPage and assign their values.
	realPage = new Page*[rpSize];
	for(CARD32 i = 0; i < rpSize; i++) {
		realPage[i] = (Page *)(pages + i * PageSize);
	}

	MapFlags vacant = {6};
	MapFlags clear = {0};

	maps = new Map[vpSize];
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
	for(CARD32 i = 256; i < rpSize; i++) {
		maps[i].mf = clear;
		maps[i].rp = rp++;
	}
	if (rp != rpSize) ERROR();
	// vp: [rpSize .. vpSize)
	for(CARD32 i = rpSize; i < vpSize; i++) {
		maps[i].mf = vacant;
		maps[i].rp = 0;
	}

	// initialize related class
	PageCache::initialize();
}

void Memory::finalize() {
	delete [] maps;
	maps = 0;

	delete [] realPage;
	realPage = 0;

	delete[] pages;
	pages = 0;

	vpSize = rpSize = 0;
}

void Memory::reserveDisplayPage(CARD16 displayWidth_, CARD16 displayHeight_) {
	// Taken from APilot/15.3/Faces/Private/UserterminalHeadGuam.mesa
	// UserTerminal::CalculateDisplayPages
    // wordsPerDWord: CARDINAL = 2;
    // bitsPerDWord: CARDINAL = Environment.bitsPerWord * wordsPerDWord;
    const int wordsPerDWord = 2;
	const int bitsPerDWord = Environment::bitsPerWord * wordsPerDWord;

	displayWidth = displayWidth_;
	displayHeight = displayHeight_;

	const int alignedDisplayWidth = ((displayWidth + bitsPerDWord - 1) / bitsPerDWord) * bitsPerDWord;
	displayBytesPerLine = alignedDisplayWidth / 8;

	const int PAGE_SIZE = PageSize * sizeof(CARD16);
	const int imageSize = (alignedDisplayWidth * displayHeight) / 8;
	displayPageSize     = ((imageSize + PAGE_SIZE - 1) / PAGE_SIZE);

	const CARD32 vp = rpSize - displayPageSize;
	displayRealPage = maps[vp].rp;

	// make [rpSize - displayPageSize..rpSize) vacant
	MapFlags vacant = {6};
	for(CARD32 i = 0; i < displayPageSize; i++) {
		Map map;
		map.mf = vacant;
		map.rp = 0;
		WriteMap(vp + i, map);
	}

	logger.info("%s rp = %6X+%2X", __FUNCTION__, rpSize - displayPageSize, displayPageSize);
}
void Memory::mapDisplay(CARD32 vp, CARD32 rp, CARD32 pageCount) {
	logger.info("%s  %6X+%2X  %6X %3d", __FUNCTION__, vp, pageCount, rp, pageCount);
	if (rp != displayRealPage) {
		logger.fatal("rp              = %d", rp);
		logger.fatal("displayRealPage = %d", displayRealPage);
		ERROR();
	}
	if (pageCount != displayPageSize) {
		logger.fatal("pageCount       = %d", pageCount);
		logger.fatal("displayPageSize = %d", displayPageSize);
		ERROR();
	}
	displayVirtualPage = vp;

	// make [vp..vp + pageCount) clear
	MapFlags clear = {0};
	for(CARD32 i = 0; i < pageCount; i++) {
		Map map;
		map.mf = clear;
		map.rp = rp + i;
		WriteMap(vp + i, map);
	}
}

uint64_t        PageCache::missConflict;
uint64_t        PageCache::missEmpty;
uint64_t        PageCache::hit;
PageCache::Entry PageCache::entry[N_ENTRY];


CARD16* Memory::Fetch(CARD32 virtualAddress) {
	PERF_COUNT(memory, MemoryFetch)
	const CARD32 vp = virtualAddress / PageSize;
	const CARD32 of = virtualAddress % PageSize;
	if (vpSize <= vp) {
		logger.fatal("virtaulAddress = %08X  vp = %08X", virtualAddress, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	if (Vacant(mf)) PageFault(virtualAddress);
	if (mf.referenced == 0) {
		mf.referenced = 1;
		p->mf = mf;
	}
	Page* page = realPage[p->rp];
	if (page == 0) ERROR();
	//
	return page->word + of;
}
CARD16* Memory::Store(CARD32 virtualAddress) {
	PERF_COUNT(memory, MemoryStore)
	const CARD32 vp = virtualAddress / PageSize;
	const CARD32 of = virtualAddress % PageSize;
	if (vpSize <= vp) {
		logger.fatal("virtaulAddress = %08X  vp = %08X", virtualAddress, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	if (Vacant(mf)) PageFault(virtualAddress);
	if (Protect(mf)) WriteProtectFault(virtualAddress);
	if (mf.referenced == 0 || mf.dirty == 0) {
		mf.referenced = 1;
		mf.dirty      = 1;
		p->mf = mf;
	}
	Page* page = realPage[p->rp];
	if (page == 0) ERROR();
	//
	return page->word + of;
}
CARD16* Memory::getAddress(CARD32 virtualAddress) {
	PERF_COUNT(memory, GetAddress)
	const CARD32 vp = virtualAddress / PageSize;
	const CARD32 of = virtualAddress % PageSize;
	if (vpSize <= vp) {
		logger.fatal("%s  va = %6X  vp = %4X", __FUNCTION__, virtualAddress, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	if (Vacant(mf)) {
		logger.fatal("%s  va = %6X  vp = %4X", __FUNCTION__, virtualAddress, vp);
		logger.fatal("%s  mf = %4X  rp = %4X", __FUNCTION__, maps[vp].mf.u + 0, maps[vp].rp + 0);
		ERROR();
	}
	Page* page = realPage[p->rp];
	if (page == 0) ERROR();
	//
	return page->word + of;
}
int Memory::isVacant(CARD32 virtualAddress) {
	const CARD32 vp = virtualAddress / PageSize;
	if (vpSize <= vp) {
		logger.fatal("%s  va = %6X  vp = %4X", __FUNCTION__, virtualAddress, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	return Vacant(mf);
}
void Memory::setReferencedFlag(CARD32 vp) {
	if (vpSize <= vp) {
		logger.fatal("%s  vp = %4X", __FUNCTION__, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	mf.referenced = 1;
	p->mf = mf;
}
void Memory::setReferencedDirtyFlag(CARD32 vp) {
	if (vpSize <= vp) {
		logger.fatal("%s  vp = %4X", __FUNCTION__, vp);
		ERROR();
	}
	Map *p = maps + vp;
	MapFlags mf = p->mf;
	mf.referenced = 1;
	mf.dirty      = 1;
	p->mf = mf;
}

Memory::Map Memory::ReadMap(CARD32 vp) {
	if (vpSize <= vp) ERROR();
	Map map = maps[vp];
	if (Vacant(map.mf)) map.rp = 0;
	return map;
}
void Memory::WriteMap(CARD32 vp, Map map) {
	if (vpSize <= vp) {
		logger.error("vpSize  %d", vpSize);
		logger.error("vp      %d", vp);
		logger.error("map     %02X", map);
		ERROR();
	}
	if (rpSize <= map.rp) ERROR();

	if (Vacant(map.mf)) map.rp = 0;
	maps[vp] = map;
	PERF_COUNT(memory, WriteMap)
	PageCache::invalidate(vp);
}

void PageCache::fetchSetup(Entry *p, CARD32 vp) {
	if (PERF_ENABLE) {
		if (p->vpno) missConflict++;
		else missEmpty++;
	}
	// Overwrite content of entry
	p->page      = Memory::Fetch(vp * PageSize);
	// NO PAGE FAULT AFTER HERE
	p->vpno      = vp;
	p->flagFetch = 1;
	p->flagStore = 0;
}
void PageCache::fetchMaintainFlag(Entry *p, CARD32 vp) {
	Memory::setReferencedFlag(vp);
	p->flagFetch = 1;
	// FIXME Should p->flagStore set to 0?
	//   There can be a chance that fetch after store in same page.
	// p->flagStore = 0;
}
void PageCache::storeSetup(Entry *p, CARD32 vp) {
	if (PERF_ENABLE) {
		if (p->vpno) missConflict++;
		else missEmpty++;
	}
	// Overwrite content of entry
	p->page      = Memory::Store(vp * PageSize);
	// NO PAGE FAULT AFTER HERE
	p->vpno      = vp;
	p->flagFetch = 1;
	p->flagStore = 1;
}
void PageCache::storeMaintainFlag(Entry *p, CARD32 vp) {
	Memory::setReferencedDirtyFlag(vp);
	p->flagFetch = 1;
	p->flagStore = 1;
}

void PageCache::stats() {
	int used = 0;
	for(CARD32 i = 0; i < N_ENTRY; i++) {
		if (entry[i].vpno) used++;
	}

	if (PERF_ENABLE) {
		uint64_t total = (missEmpty + missConflict) + hit;
		logger.info("PageCache %5d / %5d  %10llu %6.2f%%   miss empty %10llu  conflict %10llu", used, N_ENTRY, total, ((double)hit / total) * 100.0, missEmpty, missConflict);
	} else {
		logger.info("PageCache %5d / %5d", used, N_ENTRY);
	}
}
