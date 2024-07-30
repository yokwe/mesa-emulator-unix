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


//
// Memory
//

static const CARD16 DEFAULT_IO_REGION_PAGE   =  128; // FIXME See Agent::ioRegionPage
void MemoryInit(CARD32 vmBits, CARD32 rmBits, CARD16 ioRegionPage = DEFAULT_IO_REGION_PAGE);

static inline int IsSamePage(CARD32 vaA, CARD32 vaB) {
	return (vaA / PageSize) == (vaB / PageSize);
}


//
// Memory Map
//
void WriteMap(CARD32 vp, MapFlags flag, CARD32 rp);
std::pair<MapFlags, CARD32> ReadMap(CARD32 vp);


//
// Access Memory
//
CARD16* GetAddress(CARD32 va);
CARD16* Fetch(CARD32 va);
CARD16* Store(CARD32 va);

static inline CARD16* FetchNext(CARD32 va, CARD16* p0) {
	return (va & PageMask) == (PageSize - 1) ? Fetch(va + 1) : p0 + 1;
}
static inline CARD16* StoreNext(CARD32 va, CARD16* p0) {
	return (va & PageMask) == (PageSize - 1) ? Store(va + 1) : p0 + 1;
}
static inline CARD32 ReadDbl(CARD32 va) {
	CARD16* p0 = Fetch(va);
	CARD16* p1 = FetchNext(va, p0);
	return (*p1 << WordSize) | *p0;
}


//
// MDS
//
static inline CARD32 LengthenPointer(CARD16 ptr) {
	return MDS + ptr;
}
static inline CARD16* GetAddressMDS(CARD16 ptr) {
	return GetAddress(LengthenPointer(ptr));
}
static inline CARD16* FetchMDS(CARD16 ptr) {
	return GetAddressMDS(ptr);
}
static inline CARD16* StoreMDS(CARD16 ptr) {
	return GetAddressMDS(ptr);
}
static inline CARD16* FetchMDSNext(CARD32 va, CARD16* p0) {
	return (va & PageMask) == (PageSize - 1) ? FetchMDS(va + 1) : p0 + 1;
}
static inline CARD16* StoreMDSNext(CARD32 va, CARD16* p0) {
	return (va & PageMask) == (PageSize - 1) ? StoreMDS(va + 1) : p0 + 1;
}
static inline CARD32 ReadDblMDS(CARD16 ptr) {
	CARD16* p0 = FetchMDS(ptr);
	CARD16* p1 = FetchMDSNext(ptr, p0);
	return (*p1 << WordSize) | *p0;
}


//
// Code
//

// 3.1.4.3 Code Segments
static inline CARD16 ReadCode(CARD16 offset) {
	return *Fetch(CB + offset);
}

// 4.3 Instruction Fetch
static inline CARD8 GetCodeByte() {
	CARD16 word = ReadCode(PC / 2);
	// NO PAGE FAULT AFTER HERE
	return (PC++ & 1) ? LowByte(word) : HighByte(word);
}
static inline CARD16 GetCodeWord() {
	CARD32 ptr = CB + (PC / 2);
	CARD16* p0 = Fetch(ptr + 0);
	if (PC & 1) {
		// PC is odd
		CARD16* p1 = (ptr & PageMask) == (PageSize - 1) ? Fetch(ptr + 1) : p0 + 1;
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
