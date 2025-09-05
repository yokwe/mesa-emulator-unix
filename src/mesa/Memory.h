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

#pragma once

#include "../util/Util.h"

#include "Constant.h"
#include "Type.h"
#include "Pilot.h"

#include "Variable.h"

#include "../util/Perf.h"


// 3.1 Virtual Memory
class Memory {
public:
	static const CARD32 MAX_REALMEMORY_PAGE_SIZE = RealMemoryImplGuam::largestArraySize * WordSize;

	struct Page { CARD16 word[PageSize]; };
	struct Map {
		MapFlags mf;
		CARD16 rp;
	} __attribute__((packed));

	static Map     ReadMap(CARD32 vp);
	static void    WriteMap(CARD32 vp, Map map);
	static CARD16* Fetch(CARD32 virtualAddress);
	static CARD16* Store(CARD32 virtualAddress);

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

	static void initialize(int vmBits, int rmBits, CARD16 ioRegionPage);

	static void finalize();

	static CARD16 *getAddress(CARD32 virtualAddress);
	static int isVacant(CARD32 virtualAddress);
	//
	static void setReferencedFlag(CARD32 vp);
	static void setReferencedDirtyFlag(CARD32 vp);

	static inline CARD32 getVPSize() {
		return vpSize;
	}
	static inline CARD32 getRPSize() {
		return rpSize - displayPageSize;
	}

	static void reserveDisplayPage(CARD16 displayWidth_, CARD16 displayHeight_);
	static inline CARD32 getDisplayPageSize() {
		if (displayRealPage == 0) ERROR();
		return displayPageSize;
	}
	static inline CARD32 getDisplayBytesPerLine() {
		if (displayRealPage == 0) ERROR();
		return displayBytesPerLine;
	}

	static void mapDisplay(CARD32 vp, CARD32 rp, CARD32 pageCount);
	static inline CARD32 getDisplayRealPage() {
		if (displayRealPage == 0) ERROR();
		return displayRealPage;
	}
	static inline Page* getDisplayPage() {
		if (displayRealPage == 0) ERROR();
		return realPage[displayRealPage];
	}
	static inline CARD32 getDisplayVirtualPage() {
		if (displayVirtualPage == 0) ERROR();
		return displayVirtualPage;
	}
	static inline int isDisplayPage(CARD32 vp) {
		if (displayVirtualPage == 0) ERROR();
		return (displayVirtualPage <= vp && vp < (displayVirtualPage + displayPageSize));
	}

private:
	static CARD32  vpSize;
	static CARD32  rpSize;
	static Map    *maps;
	static CARD16 *pages;
	static Page  **realPage;
	static CARD32  displayPageSize;
	static CARD32  displayRealPage;
	static CARD32  displayVirtualPage;
	static CARD32  displayWidth;
	static CARD32  displayHeight;
	static CARD32  displayBytesPerLine;
	static CARD32  mds;
};


class PageCache {
protected:
	static constexpr CARD32 N_BIT = 16;
	static constexpr CARD32 N_ENTRY = 1 << N_BIT;
	static constexpr CARD32 MASK = (1 << N_BIT) - 1;
	static inline CARD32 hash(CARD32 vp_) {
		// When N_BIT == 16, there is no conflict during booting gvwin
		return vp_ & MASK;
	}
	//
	struct Entry {
		union {
			CARD32 flag;
			struct {
				CARD32 vpno      : 30;
				CARD32 flagFetch :  1;
				CARD32 flagStore :  1;
			};
		};
		CARD16* page;
	};
	static uint64_t   hit;
	static uint64_t   missConflict;
	static uint64_t   missEmpty;
	static Entry       entry[N_ENTRY];

public:
	static void initialize() {
		for(CARD32 i = 0; i < N_ENTRY; i++) {
			entry[i].flag = 0;
			entry[i].page = 0;
		}
		hit          = 0;
		missEmpty    = 0;
		missConflict = 0;
	}
	static inline void invalidate(CARD32 vp_) {
		Entry *p = entry + hash(vp_);
		if (p->vpno == vp_) {
			// void entry of vp_
			p->flag = 0;
			p->page = 0;
		}
	}
	static void stats();

	static void fetchSetup(Entry *p, CARD32 vp);
	static void fetchMaintainFlag(Entry *p, CARD32 vp);
	__attribute__((always_inline)) static inline CARD16* fetch(CARD32 va) {
		const CARD32 vp = va / PageSize;
		const CARD32 of = va % PageSize;

		Entry *p = entry + hash(vp);
		if (p->vpno != vp) {
			fetchSetup(p, vp);
		} else {
			if (PERF_ENABLE) hit++;
			if (p->flagFetch == 0) fetchMaintainFlag(p, vp);
		}
		return p->page + of;
	}
	static void storeSetup(Entry *p, CARD32 vp);
	static void storeMaintainFlag(Entry *p, CARD32 vp);
	__attribute__((always_inline)) static inline CARD16* store(CARD32 va) {
		const CARD32 vp = va / PageSize;
		const CARD32 of = va % PageSize;

		Entry *p = entry + hash(vp);
		if (p->vpno != vp) {
			storeSetup(p, vp);
		} else {
			if (PERF_ENABLE) hit++;
			if (p->flagStore == 0) storeMaintainFlag(p, vp);
		}
		return p->page + of;
	}
};

// 3.1.3 Virtual Memory Access
__attribute__((always_inline)) static inline CARD16* Fetch(CARD32 virtualAddress) {
	PERF_COUNT(memory, Fetch)
	return PageCache::fetch(virtualAddress);
}
__attribute__((always_inline)) static inline CARD16* Store(CARD32 virtualAddress) {
	PERF_COUNT(memory, Store)
	return PageCache::store(virtualAddress);
}
__attribute__((always_inline)) static inline int isSamePage(CARD32 ptrA, CARD32 ptrB) {
	return (ptrA / PageSize) == (ptrB / PageSize);
}
__attribute__((always_inline)) static inline CARD32 ReadDbl(CARD32 virtualAddress) {
	PERF_COUNT(memory, ReadDbl)
	CARD16* p0 = PageCache::fetch(virtualAddress + 0);
	CARD16* p1 = isSamePage(virtualAddress + 0, virtualAddress + 1) ? (p0 + 1) : PageCache::fetch(virtualAddress + 1);
//	Long t;
//	t.low  = *p0;
//	t.high = *p1;
//	return t.u;
	return (*p1 << WordSize) | *p0;
}

// 3.2.1 Main Data Space Access
__attribute__((always_inline)) static inline CARD16* FetchMds(CARD16 ptr) {
	PERF_COUNT(memory, FetchMds)
	return PageCache::fetch(LengthenPointer(ptr));
}
__attribute__((always_inline)) static inline CARD16* StoreMds(CARD16 ptr) {
	PERF_COUNT(memory, StoreMds)
	return PageCache::store(LengthenPointer(ptr));
}
__attribute__((always_inline)) static inline CARD32 ReadDblMds(CARD16 ptr) {
	PERF_COUNT(memory, ReadDblMds)
	CARD16* p0 = PageCache::fetch(LengthenPointer(ptr + 0));
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : PageCache::fetch(LengthenPointer(ptr + 1));
//	Long t;
//	t.low  = *p0;
//	t.high = *p1;
//	return t.u;
	return (*p1 << WordSize) | *p0;
}

// 3.1.4.3 Code Segments
__attribute__((always_inline)) static inline CARD16 ReadCode(CARD16 offset) {
	return *PageCache::fetch(CB + offset);
}

// 4.3 Instruction Fetch
__attribute__((always_inline)) static inline CARD8 GetCodeByte() {
	PERF_COUNT(memory, GetCodeByte)
	CARD16 word = ReadCode(PC / 2);
	// NO PAGE FAULT AFTER HERE
	return (PC++ & 1) ? LowByte(word) : HighByte(word);
}
__attribute__((always_inline)) static inline CARD16 GetCodeWord() {
	PERF_COUNT(memory, GetCodeWord)
	CARD32 ptr = CB + (PC / 2);
	CARD16* p0 = PageCache::fetch(ptr + 0);
	if (PC & 1) {
		// PC is odd
		CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : PageCache::fetch(ptr + 1);
		// NO PAGE FAULT AFTER HERE
		PC += 2;
		return (LowByte(*p0) << 8) | HighByte(*p1);
	} else {
		// NO PAGE FAULT AFTER HERE
		PC += 2;
		return *p0;
	}
}

// 7.4 String Instructions
static inline BYTE FetchByte(LONG_POINTER ptr, LONG_CARDINAL offset) {
	PERF_COUNT(memory, FetchByte)
	ptr += offset / 2;
	BytePair word = {*Fetch(ptr)};
	return ((offset % 2) == 0) ? (BYTE)word.left : (BYTE)word.right;
}
static inline CARD16 FetchWord(LONG_POINTER ptr, LONG_CARDINAL offset) {
	BytePair ret;
	ret.left  = FetchByte(ptr, offset);
	ret.right = FetchByte(ptr, offset + 1);
	return ret.u;
}
static inline void StoreByte(LONG_POINTER ptr, LONG_CARDINAL offset, BYTE data) {
	PERF_COUNT(memory, StoreByte)
	ptr += offset / 2;
	CARD16* p = Store(ptr);
	BytePair word = {*p};
	if ((offset % 2) == 0) {
		word.left = data;
	} else {
		word.right = data;
	}
	*p = word.u;
}

// 7.5 Field Instruction
//const UNSPEC Field_MaskTable[WordSize] = {
//		0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
//		0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
//};
static inline CARD16 Field_MaskTable(CARD8 n) {
	return (CARD16)((1U << (n + 1)) - 1);
}

static inline UNSPEC ReadField(UNSPEC source, CARD8 spec8) {
	PERF_COUNT(memory, ReadField)
	FieldSpec spec = {spec8};

	if (WordSize < (spec.pos + spec.size + 1)) ERROR();
	int shift = WordSize - (spec.pos + spec.size + 1);
	// shift is always positive
	// return Shift(source, -shift) & MaskTable(spec.size);
	return (source >> shift) & Field_MaskTable(spec.size);
}
static inline UNSPEC WriteField(UNSPEC dest, CARD8 spec8, UNSPEC data) {
	PERF_COUNT(memory, WriteField)
	FieldSpec spec = {spec8};

	if (WordSize < (spec.pos + spec.size + 1)) ERROR();
	int shift = WordSize - (spec.pos + spec.size + 1);
	// shift is always positive
	//UNSPEC mask = Shift(MaskTable[spec.size], shift);
	UNSPEC mask = Field_MaskTable(spec.size) << shift;
	//return (dest & ~mask) | (Shift(data, shift) & mask);
	return (dest & ~mask) | ((data << shift) & mask);
}


// 9.4.2 External Function Calls
static inline CARD32 FetchLink(CARD32 offset) {
	GlobalWord word = {*Fetch(GO_OFFSET(GF, word))};
	//CARD32 pointer = word.codelinks ? (CB - (CARD32)((offset + 1) * 2)) : (GlobalBase(GF) - (CARD32)((offset + 1) * 2));
	CARD32 pointer = (word.codelinks ? CB : GlobalBase(GF)) - (CARD32)((offset + 1) * 2);
	return ReadDbl(pointer);
}

// 10.1.1 Process Data Area
__attribute__((always_inline)) static inline LONG_POINTER LengthenPdaPtr(POINTER ptr) {
	return PDA + ptr;
}
__attribute__((always_inline)) static inline POINTER OffsetPda(LONG_POINTER ptr) {
	if ((ptr & 0xffff0000) != (PDA & 0xffff0000)) ERROR();
	return (CARD16)(ptr - PDA);
}

static inline CARD16* FetchPda(POINTER ptr) {
	PERF_COUNT(memory, FetchPda)
	return PageCache::fetch(LengthenPdaPtr(ptr));
}
static inline CARD16* StorePda(POINTER ptr) {
	PERF_COUNT(memory, StorePda)
	return PageCache::store(LengthenPdaPtr(ptr));
}
