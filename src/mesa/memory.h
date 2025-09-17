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
 // memory.h
 //

#pragma once

#include "../util/Util.h"

#include "Constant.h"
#include "Type.h"

#include "Variable.h"

#include "../util/Perf.h"


// 3.1 Virtual Memory
namespace memory {
	struct Page { CARD16 word[PageSize]; };
#pragma pack(push, 2)
	struct Map {
		MapFlags mf;
		CARD16 rp;
	};
#pragma pack(pop)

	Map     ReadMap(CARD32 vp);
	void    WriteMap(CARD32 vp, Map map);

	CARD16* FetchPage(CARD32 vp);
	CARD16* StorePage(CARD32 vp);

	void    initialize(int vmBits, int rmBits, CARD16 ioRegionPage);
	void    finalize();

	CARD16* peek(CARD32 va);
	//
	void    setReferencedFlag(CARD32 vp);
	void    setReferencedDirtyFlag(CARD32 vp);

	CARD32  getVPSize();
	CARD32  getRPSize();

	void    reserveDisplayPage(CARD16 displayWidth_, CARD16 displayHeight_);
	CARD32  getDisplayPageSize();
	CARD32  getDisplayBytesPerLine();

	void    mapDisplay(CARD32 vp, CARD32 rp, CARD32 pageCount);
	CARD32  getDisplayRealPage();
	Page*   getDisplayPage();
	CARD32  getDisplayVirtualPage();

	bool	isMemoryInitialize();
	bool    isDisplayMapped();

	int     isDisplayPage(CARD32 vp);

	namespace cache {
		constexpr CARD32 N_BIT = 16;
		constexpr CARD32 N_ENTRY = 1 << N_BIT;
		constexpr CARD32 MASK = (1 << N_BIT) - 1;
		inline CARD32 hash(CARD32 vp_) {
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

			void clear() {
				flag = 0;
				page = 0;
			}
		};
		extern uint64_t   hit;
		extern uint64_t   missConflict;
		extern uint64_t   missEmpty;
		extern Entry      entry[N_ENTRY];

		inline Entry* getEntry(CARD32 vp_) {
			return entry + hash(vp_);
		}

		void initialize();
		void invalidate(CARD32 vp_);
		void stats();

		void fetchSetup(Entry *p, CARD32 vp);
		void fetchMaintainFlag(Entry *p, CARD32 vp);
		inline CARD16* fetch(CARD32 va) {
			const CARD32 vp = va / PageSize;
			Entry *p = getEntry(vp);
			if (p->vpno != vp) {
				fetchSetup(p, vp);
			} else {
				if (PERF_ENABLE) hit++;
				if (p->flagFetch == 0) fetchMaintainFlag(p, vp);
			}
			return p->page + (va % PageSize);
		}

		void storeSetup(Entry *p, CARD32 vp);
		void storeMaintainFlag(Entry *p, CARD32 vp);
		inline CARD16* store(CARD32 va) {
			const CARD32 vp = va / PageSize;
			Entry *p = getEntry(vp);
			if (p->vpno != vp) {
				storeSetup(p, vp);
			} else {
				if (PERF_ENABLE) hit++;
				if (p->flagStore == 0) storeMaintainFlag(p, vp);
			}
			return p->page + (va % PageSize);
		}
	}
}


inline int isSamePage(CARD32 ptrA, CARD32 ptrB) {
	return (ptrA / PageSize) == (ptrB / PageSize);
}
inline int isLastOfPage(CARD32 ptr) {
	return (ptr & PageOffset) == PageOffset;
}

// 3.1.3 Virtual Memory Access
inline CARD16* Fetch(CARD32 virtualAddress) {
	PERF_COUNT(memory, Fetch)
	return memory::cache::fetch(virtualAddress);
}
inline CARD16* Store(CARD32 virtualAddress) {
	PERF_COUNT(memory, Store)
	return memory::cache::store(virtualAddress);
}
inline CARD32 ReadDbl(CARD32 virtualAddress) {
	PERF_COUNT(memory, ReadDbl)
	CARD16* p0 = memory::cache::fetch(virtualAddress + 0);
	CARD16* p1 = p0 + 1;
	if (isLastOfPage(virtualAddress)) p1 = memory::cache::fetch(virtualAddress + 1);
//	Long t;
//	t.low  = *p0;
//	t.high = *p1;
//	return t.u;
	return (*p1 << WordSize) | *p0;
}

// 3.2.1 Main Data Space Access
inline CARD16* FetchMds(CARD16 ptr) {
	PERF_COUNT(memory, FetchMds)
	return memory::cache::fetch(LengthenPointer(ptr));
}
inline CARD16* StoreMds(CARD16 ptr) {
	PERF_COUNT(memory, StoreMds)
	return memory::cache::store(LengthenPointer(ptr));
}
inline CARD32 ReadDblMds(CARD16 ptr) {
	PERF_COUNT(memory, ReadDblMds)
	CARD32 p = LengthenPointer(ptr);
	CARD16* p0 = memory::cache::fetch(p + 0);
	CARD16* p1 = p0 + 1;
	if (isLastOfPage(p)) p1 = memory::cache::fetch(p + 1);
//	Long t;
//	t.low  = *p0;
//	t.high = *p1;
//	return t.u;
	return (*p1 << WordSize) | *p0;
}

// 3.1.4.3 Code Segments
inline CARD16 ReadCode(CARD16 offset) {
	return *memory::cache::fetch(CB + offset);
}

// 4.3 Instruction Fetch
inline CARD8 GetCodeByte() {
	PERF_COUNT(memory, GetCodeByte)
	CARD16 word = ReadCode(PC / 2);
	// NO PAGE FAULT AFTER HERE
	return (PC++ & 1) ? LowByte(word) : HighByte(word);
}
inline CARD16 GetCodeWord() {
	PERF_COUNT(memory, GetCodeWord)
	CARD32 ptr = CB + (PC / 2);
	CARD16* p0 = memory::cache::fetch(ptr + 0);
	if (PC & 1) {
		// PC is odd
		CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : memory::cache::fetch(ptr + 1);
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
inline BYTE FetchByte(LONG_POINTER ptr, LONG_CARDINAL offset) {
	PERF_COUNT(memory, FetchByte)
	ptr += offset / 2;
	BytePair word = {*Fetch(ptr)};
	return ((offset % 2) == 0) ? (BYTE)word.left : (BYTE)word.right;
}
inline CARD16 FetchWord(LONG_POINTER ptr, LONG_CARDINAL offset) {
	BytePair ret;
	ret.left  = FetchByte(ptr, offset);
	ret.right = FetchByte(ptr, offset + 1);
	return ret.u;
}
inline void StoreByte(LONG_POINTER ptr, LONG_CARDINAL offset, BYTE data) {
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
inline CARD16 Field_MaskTable(CARD8 n) {
	return (CARD16)((1U << (n + 1)) - 1);
}

inline UNSPEC ReadField(UNSPEC source, CARD8 spec8) {
	PERF_COUNT(memory, ReadField)
	FieldSpec spec = {spec8};

	if (WordSize < (spec.pos + spec.size + 1)) ERROR();
	int shift = WordSize - (spec.pos + spec.size + 1);
	// shift is always positive
	// return Shift(source, -shift) & MaskTable(spec.size);
	return (source >> shift) & Field_MaskTable(spec.size);
}
inline UNSPEC WriteField(UNSPEC dest, CARD8 spec8, UNSPEC data) {
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
inline CARD32 FetchLink(CARD32 offset) {
	GlobalWord word = {*Fetch(GO_OFFSET(GF, word))};
	//CARD32 pointer = word.codelinks ? (CB - (CARD32)((offset + 1) * 2)) : (GlobalBase(GF) - (CARD32)((offset + 1) * 2));
	CARD32 pointer = (word.codelinks ? CB : GlobalBase(GF)) - (CARD32)((offset + 1) * 2);
	return ReadDbl(pointer);
}

// 10.1.1 Process Data Area
inline LONG_POINTER LengthenPdaPtr(POINTER ptr) {
	return PDA + ptr;
}
inline POINTER OffsetPda(LONG_POINTER ptr) {
	if ((ptr & 0xffff0000) != (PDA & 0xffff0000)) ERROR();
	return (CARD16)(ptr - PDA);
}

inline CARD16* FetchPda(POINTER ptr) {
	PERF_COUNT(memory, FetchPda)
	return memory::cache::fetch(LengthenPdaPtr(ptr));
}
inline CARD16* StorePda(POINTER ptr) {
	PERF_COUNT(memory, StorePda)
	return memory::cache::store(LengthenPdaPtr(ptr));
}
