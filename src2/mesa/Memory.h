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

#pragma once

#include <utility>

#include "Type.h"
#include "Constant.h"
#include "Variable.h"
#include "Function.h"

namespace mesa {

//
// Stack
//
#define STACK_ERROR() { \
	logger.fatal("STACK_ERROR  %s -- %5d %s", __FUNCTION__, __LINE__, __FILE__); \
	StackError(); \
}
// 3.3.2 Evaluation Stack
static inline CARD32 StackCount() {
	return SP;
}
static inline void Push(CARD16 data) {
	if (SP == StackDepth) StackError();
	stack[SP++] = data;
}
static inline CARD16 Pop() {
	if (SP == 0) StackError();
	return stack[--SP];
}
// Note that double-word quantities are placed on the stack so that
// the least significant word is at the lower-numbered stack index.
static inline void PushLong(CARD32 data) {
//	Long t = {data};
//	Push(t.low);
//	Push(t.high);
	Push((CARD16)data);
	Push((CARD16)(data >> WordSize));
}
static inline CARD32 PopLong() {
//	Long t;
//	t.high = Pop();
//	t.low = Pop();
//	return t.u;
	CARD32 ret = Pop() << WordSize;
	return ret | Pop();
}
//static inline void MinimalStack() {
//	if (SP != 0) STACK_ERROR();
//}
#define MINIMAL_STACK() { \
	if (SP != 0) { \
		logger.fatal("MINIMAL_STACK  %s -- %5d %s", __FUNCTION__, __LINE__, __FILE__); \
		STACK_ERROR(); \
	} \
}

static inline void Recover() {
	if (SP == StackDepth) StackError();
	SP++;
}
static inline void Discard() {
	if (SP == 0) StackError();
	SP--;
}


class Memory {
public:
	static const CARD32 MAX_REALMEMORY_PAGE_SIZE = RealMemoryImplGuam::largestArraySize;
	static const CARD16 DEFAULT_IO_REGION_PAGE   =  128; // FIXME See Agent::ioRegionPage

	static const CARD32 VMBITS_MIN = 20;
	static const CARD32 VMBITS_MAX = 25;

	static const CARD32 RMBITS_MIN = 20;
	static const CARD32 MRBITS_MAX = 24;


	Memory(): vpSize(0), rpSize(0), memoryArray(0), flagArray(0), pageArray(0), mds(0) {
		 //
	}

	void initialize(CARD32 vmBits, CARD32 rmBits, CARD16 ioRegionPage = DEFAULT_IO_REGION_PAGE);

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


static inline int isSamePage(CARD32 vaA, CARD32 vaB) {
	return (vaA / PageSize) == (vaB / PageSize);
}

static inline void WriteMap(CARD32 vp, MapFlags flag, CARD32 rp) {
	memory.writeMap(vp, flag, rp);
}
static inline std::pair<MapFlags, CARD32> ReadMap(CARD32 vp) {
	return memory.readMap(vp);
}


//
// Access Memory
//
static inline CARD16* Fetch(CARD32 va) {
	return memory.fetch(va);
}
static inline CARD16* Store(CARD32 va) {
	return memory.store(va);
}
static inline CARD32 ReadDbl(CARD32 va) {
	CARD16* p0 = memory.fetch(va + 0);
	CARD16* p1 = (va & PageMask) == (PageSize - 1) ? memory.fetch(va + 1) : p0 + 1;
	return (*p1 << WordSize) | *p0;
}


//
// MDS
//
static inline void setMDS(CARD32 va) {
	memory.setMDS(va);
}
static inline CARD32 getMDS() {
	return memory.getMDS();
}
static inline CARD32 lengthenPointer(CARD16 ptr) {
	return memory.lengthenPointer(ptr);
}


//
// Access MDS memory
// No need to maintain flags
//
static inline CARD16* FetchMDS(CARD16 ptr) {
	return memory.getAddressMDS(ptr);
}
static inline CARD16* StoreMDS(CARD16 ptr) {
	return memory.getAddressMDS(ptr);
}
static inline CARD32 ReadDblMDS(CARD16 ptr) {
	CARD16* p0 = memory.getAddressMDS(ptr + 0);
	CARD16* p1 = (ptr & PageMask) == (PageSize - 1) ? memory.getAddressMDS(ptr + 1) : p0 + 1;
	return (*p1 << WordSize) | *p0;
}


//
// Code
//

// 3.1.4.3 Code Segments
static inline CARD16 ReadCode(CARD16 offset) {
	return *memory.fetch(CB + offset);
}

// 4.3 Instruction Fetch
static inline CARD8 GetCodeByte() {
	CARD16 word = ReadCode(PC / 2);
	// NO PAGE FAULT AFTER HERE
	return (PC++ & 1) ? LowByte(word) : HighByte(word);
}
static inline CARD16 GetCodeWord() {
	CARD32 ptr = CB + (PC / 2);
	CARD16* p0 = memory.fetch(ptr + 0);
	if (PC & 1) {
		// PC is odd
		CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : memory.fetch(ptr + 1);
		// NO PAGE FAULT AFTER HERE
		PC += 2;
		return (LowByte(*p0) << 8) | HighByte(*p1);
	} else {
		// NO PAGE FAULT AFTER HERE
		PC += 2;
		return *p0;
	}
}


}
