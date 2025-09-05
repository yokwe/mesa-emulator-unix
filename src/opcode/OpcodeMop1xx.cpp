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
// OpcodeMop1xx.cpp
//

#include "../util/Debug.h"
#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Memory.h"

#include "Opcode.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_R_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  R %3d", savedPC, arg);
	POINTER ptr = Pop();
	CARD16* p = FetchMds(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
#define Rn(n) \
void E_R##n () { \
	E_R_(n); \
}
// 0100  ASSIGN_MOP(z, R0)
Rn(0)
// 0101  ASSIGN_MOP(z, R1)
Rn(1)
// 0102  ASSIGN_MOP(z, RB)
void E_RB() {
	E_R_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RL %3d", savedPC, arg);
	LONG_POINTER ptr = PopLong();
	CARD16* p = Fetch(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
// 0103  ASSIGN_MOP(z, RL0)
void E_RL0() {
	E_RL_(0);
}
// 0104  ASSIGN_MOP(z, RLB)
void E_RLB() {
	E_RL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RD_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RD %3d", savedPC, arg);
	POINTER ptr = Pop() + arg;
	CARD16* p0 = FetchMds(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : FetchMds(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	UNSPEC u = *p0;
	UNSPEC v = *p1;
	Push(u);
	Push(v);
}
// 0105  ASSIGN_MOP(z, RD0)
void E_RD0() {
	E_RD_(0);
}
// 0106  ASSIGN_MOP(z, RDB)
void E_RDB() {
	E_RD_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RDL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PDL %3d", savedPC, arg);
	LONG_POINTER ptr = PopLong() + arg;
	CARD16* p0 = Fetch(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Fetch(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	UNSPEC u = *p0;
	UNSPEC v = *p1;
	Push(u);
	Push(v);
}
// 0107  ASSIGN_MOP(z, RDL0)
void E_RDL0() {
	E_RDL_(0);
}
// 0110  ASSIGN_MOP(z, RDLB)
void E_RDLB() {
	E_RDL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_W_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  W %3d", savedPC, arg);
	POINTER ptr = Pop();
	CARD16* p = StoreMds(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	*p = Pop();
}
// 0111  ASSIGN_MOP(z, W0)
void E_W0() {
	E_W_(0);
}
// 0112  ASSIGN_MOP(z, WB)
void E_WB() {
	E_W_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PS_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PS %3d", savedPC, arg);
	UNSPEC u = Pop();
	POINTER ptr = Pop();
	CARD16* p = StoreMds(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	*p = u;
	SP++; //Recover();
}
// 0113  ASSIGN_MOP(z, PSB)
void E_PSB() {
	E_PS_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WL %3d", savedPC, arg);
	LONG_POINTER ptr = PopLong();
	CARD16* p = Store(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	*p = Pop();
}
// 0114  ASSIGN_MOP(z, WLB)
void E_WLB() {
	E_WL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PSL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PSL %3d", savedPC, arg);
	UNSPEC u = Pop();
	LONG_POINTER ptr = PopLong();
	CARD16* p = Store(ptr + arg);
	// NO PAGE FAULT AFTER HERE
	*p = u;
	SP++; //Recover();
	SP++; //Recover();
}
// 0115  ASSIGN_MOP(z, PSLB)
void E_PSLB() {
	E_PSL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WD_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WD %3d", savedPC, arg);
	POINTER ptr = Pop() + arg;
	CARD16* p0 = StoreMds(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : StoreMds(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = Pop();
	*p0 = Pop();
}
// 0116  ASSIGN_MOP(z, WDB)
void E_WDB() {
	E_WD_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PSD_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PSD %3d", savedPC, arg);
	UNSPEC v = Pop();
	UNSPEC u = Pop();
	POINTER ptr = Pop() + arg;
	CARD16* p0 = StoreMds(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : StoreMds(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = v;
	*p0 = u;
	SP++; //Recover();
}
// 0117  ASSIGN_MOP(z, PSD0)
void E_PSD0() {
	E_PSD_(0);
}
// 0120  ASSIGN_MOP(z, PSDB)
void E_PSDB() {
	E_PSD_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WDL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WDL %3d", savedPC, arg);
	LONG_POINTER ptr = PopLong() + arg;
	CARD16* p0 = Store(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Store(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = Pop();
	*p0 = Pop();
}
// 0121  ASSIGN_MOP(z, WDLB)
void E_WDLB() {
	E_WDL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PSDL_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PSDL %3d", savedPC, arg);
	UNSPEC v = Pop();
	UNSPEC u = Pop();
	LONG_POINTER ptr = PopLong() + arg;
	CARD16* p0 = Store(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Store(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = v;
	*p0 = u;
	SP++; //Recover();
	SP++; //Recover();
}
// 0122  ASSIGN_MOP(z, PSDLB)
void E_PSDLB() {
	E_PSDL_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLI_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLI %3d %3d", savedPC, arg0, arg1);
	POINTER ptr = *FetchMds(LF + arg0);
	CARD16* p = FetchMds(ptr + arg1);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
// 0123  ASSIGN_MOP(z, RLI00)
void E_RLI00() {
	E_RLI_(0, 0);
}
// 0124  ASSIGN_MOP(z, RLI01)
void E_RLI01() {
	E_RLI_(0, 1);
}
// 0125  ASSIGN_MOP(z, RLI02)
void E_RLI02() {
	E_RLI_(0, 2);
}
// 0126  ASSIGN_MOP(z, RLI03)
void E_RLI03() {
	E_RLI_(0, 3);
}
// 0127  ASSIGN_MOP(z, RLIP)
void E_RLIP() {
	NibblePair pair = {GetCodeByte()};
	E_RLI_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLIL_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLIL %3d %3d", savedPC, arg0, arg1);
	LONG_POINTER ptr = ReadDblMds(LF + arg0) + arg1;
	CARD16* p = Fetch(ptr);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
// 0130  ASSIGN_MOP(z, RLILP)
void E_RLILP() {
	NibblePair pair = {GetCodeByte()};
	E_RLIL_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLDI_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLDI %3d %3d", savedPC, arg0, arg1);
	POINTER ptr = *FetchMds(LF + arg0) + arg1;
	CARD16* p0 = FetchMds(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : FetchMds(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	UNSPEC u = *p0;
	UNSPEC v = *p1;
	Push(u);
	Push(v);
}
// 0131  ASSIGN_MOP(z, RLDI00)
void E_RLDI00() {
	E_RLDI_(0, 0);
}
// 0132  ASSIGN_MOP(z, RLDIP)
void E_RLDIP() {
	NibblePair pair = {GetCodeByte()};
	E_RLDI_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLDIL_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLDIL %3d %3d", savedPC, arg0, arg1);
	LONG_POINTER ptr = ReadDblMds(LF + arg0) + arg1;
	CARD16* p0 = Fetch(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Fetch(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	UNSPEC u = *p0;
	UNSPEC v = *p1;
	Push(u);
	Push(v);
}
// 0133  ASSIGN_MOP(z, RLDILP)
void E_RLDILP() {
	NibblePair pair = {GetCodeByte()};
	E_RLDIL_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RGI_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RGI %3d %3d", savedPC, arg0, arg1);
	CARD16* p = Fetch(GF + arg0);
	CARD16* q = FetchMds(*p + arg1);
	// NO PAGE FAULT AFTER HERE
	Push(*q);
}

// 0134  ASSIGN_MOP(z, RGIP)
void E_RGIP() {
	NibblePair pair = {GetCodeByte()};
	E_RGI_((CARD8)pair.left, (CARD8)pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RGIL_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RGIL %3d %3d", savedPC, arg0, arg1);
	LONG_POINTER ptr = ReadDbl(GF + arg0) + arg1;
	CARD16* p = Fetch(ptr);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
// 0135  ASSIGN_MOP(z, RGILP)
void E_RGILP() {
	NibblePair pair = {GetCodeByte()};
	E_RGIL_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WLI_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WLI %3d %3d", savedPC, arg0, arg1);
	POINTER ptr = *FetchMds(LF + arg0) + arg1;
	CARD16* p = StoreMds(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = Pop();
}
// 0136  ASSIGN_MOP(z, WLIP)
void E_WLIP() {
	NibblePair pair = {GetCodeByte()};
	E_WLI_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WLIL_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WLIL %3d %3d", savedPC, arg0, arg1);
	LONG_POINTER ptr = ReadDblMds(LF + arg0) + arg1;
	CARD16* p = Store(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = Pop();
}
// 0137  ASSIGN_MOP(z, WLILP)
void E_WLILP() {
	NibblePair pair = {GetCodeByte()};
	E_WLIL_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WLDIL_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WLDIL %3d %3d", savedPC, arg0, arg1);
	LONG_POINTER ptr = ReadDblMds(LF + arg0) + arg1;
	CARD16* p0 = Store(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Store(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = Pop();
	*p0 = Pop();
}
// 0140  ASSIGN_MOP(z, WLDILP)
void E_WLDILP() {
	NibblePair pair = {GetCodeByte()};
	E_WLDIL_(pair.left, pair.right);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RS_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RS %3d", savedPC, arg);
	CARDINAL index = Pop();
	LONG_POINTER ptr = LengthenPointer(Pop());
	CARD16 t = FetchByte(ptr, arg + index);
	// NO PAGE FAULT AFTER HERE
	Push(t);
}
// 0141  ASSIGN_MOP(z, RS)
void E_RS() {
	E_RS_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLS_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLS %3d", savedPC, arg);
	CARDINAL index = Pop();
	LONG_POINTER ptr = PopLong();
	CARD16 t = FetchByte(ptr, arg + index);
	// NO PAGE FAULT AFTER HERE
	Push(t);
}
// 0142  ASSIGN_MOP(z, RLS)
void E_RLS() {
	E_RLS_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WS_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WS %3d", savedPC, arg);
	CARDINAL index = Pop();
	LONG_POINTER ptr = LengthenPointer(Pop());
	BYTE data = LowByte(Pop());
	StoreByte(ptr, arg + index, data);
}
// 0143  ASSIGN_MOP(z, WS)
void E_WS() {
	E_WS_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WLS_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WLS %3d", savedPC, arg);
	CARDINAL index = Pop();
	LONG_POINTER ptr = PopLong();
	BYTE data = LowByte(Pop());
	StoreByte(ptr, arg + index, data);
}
// 0144  ASSIGN_MOP(z, WLS)
void E_WLS() {
	E_WLS_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RF_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RF %3d %3d", savedPC, arg0, arg1);
	POINTER ptr = Pop();
	CARD16* p = FetchMds(ptr + arg0);
	// NO PAGE FAULT AFTER HERE
	Push(ReadField(*p, arg1));
}
// 0145  ASSIGN_MOP(z, R0F)
void E_R0F() {
	E_RF_(0, GetCodeByte());
}
// 0146  ASSIGN_MOP(z, RF)
void E_RF() {
	FieldDesc desc = {GetCodeWord()};
	E_RF_((CARD8)desc.offset, (CARD8)desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLF_(CARD8 offset, CARD8 field) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLF %3d %3d", savedPC, offset, field);
	LONG_POINTER ptr = PopLong();
	CARD16* p = Fetch(ptr + offset);
	// NO PAGE FAULT AFTER HERE
	Push(ReadField(*p, field));
}
// 0147  ASSIGN_MOP(z, RL0F)
void E_RL0F() {
	E_RLF_(0, GetCodeByte());
}
// 0150  ASSIGN_MOP(z, RLF)
void E_RLF() {
	FieldDesc desc = {GetCodeWord()};
	E_RLF_((CARD8)desc.offset, (CARD8)desc.field);
}
// 0151  ASSIGN_MOP(z, RLFS)
void E_RLFS() {
	FieldDesc desc = {Pop()};
	E_RLF_((CARD8)desc.offset, (CARD8)desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLIPF_(CARD16 arg0, CARD16 arg1) {
	NibblePair pair = {(CARD8)arg0};
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLIPF %2d %2d %3d", savedPC, pair.left + 0, pair.right + 0, arg1);
	POINTER ptr = *FetchMds(LF + pair.left);
	CARD16* p = FetchMds(ptr + pair.right);
	// NO PAGE FAULT AFTER HERE
	Push(ReadField(*p, arg1));
}
// 0152  ASSIGN_MOP(z, RLIPF)
void E_RLIPF() {
	FieldDesc desc = {GetCodeWord()};
	E_RLIPF_((CARD8)desc.offset, (CARD8)desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RLILPF_(CARD16 arg0, CARD16 arg1) {
	NibblePair pair = {(CARD8)arg0};
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RLILPF %2d %2d %3d", savedPC, pair.left + 0, pair.right + 0, arg1);
	LONG_POINTER ptr = ReadDblMds(LF + pair.left);
	CARD16* p = Fetch(ptr + pair.right);
	// NO PAGE FAULT AFTER HERE
	Push(ReadField(*p, arg1));
}
// 0153  ASSIGN_MOP(z, RLILPF)
void E_RLILPF() {
	FieldDesc desc = {GetCodeWord()};
	E_RLILPF_((CARD8)desc.offset, (CARD8)desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WF_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WF %3d %3d", savedPC, arg0, arg1);
	POINTER ptr = Pop() + arg0;
	UNSPEC data = Pop();
	CARD16* p = StoreMds(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = WriteField(*p, arg1, data);
}
// 0154  ASSIGN_MOP(z, W0F)
void E_W0F() {
	E_WF_(0, GetCodeByte());
}
// 0155  ASSIGN_MOP(z, WF)
void E_WF() {
	FieldDesc desc = {GetCodeWord()};
	E_WF_(desc.offset, desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PSF_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PSF %3d %3d", savedPC, arg0, arg1);
	UNSPEC data = Pop();
	POINTER ptr = Pop() + arg0;
	CARD16* p = StoreMds(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = WriteField(*p, (CARD8)arg1, data);
	SP++; //Recover();
}
// 0156  ASSIGN_MOP(z, PSF)
void E_PSF() {
	FieldDesc desc = {GetCodeWord()};
	E_PSF_((CARD8)desc.offset, (CARD8)desc.field);
}
// 0157  ASSIGN_MOP(z, PS0F)
void E_PS0F() {
	E_PSF_(0, GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_WSF_(CARD16 arg0, CARD16 arg1) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  WSF %3d %3d", savedPC, arg0, arg1);
	UNSPEC data = Pop();
	POINTER ptr = Pop() + arg0;
	CARD16* p = StoreMds(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = WriteField(*p, (CARD8)arg1, data);
}
// 0160  ASSIGN_MOP(z, WS0F)
void E_WS0F() {
	E_WSF_(0, GetCodeByte());
}
__attribute__((always_inline)) static inline void E_WLF_(CARD16 offset, CARD16 field) {
	LONG_POINTER ptr = PopLong() + offset;
	UNSPEC data = Pop();
	CARD16* p = Store(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = WriteField(*p, (CARD8)field, data);
}
// 0161  ASSIGN_MOP(z, WL0F)
void E_WL0F() {
	E_WLF_(0, GetCodeByte());
}
// 0162  ASSIGN_MOP(z, WLF)
void E_WLF() {
	FieldDesc desc = {GetCodeWord()};
	E_WLF_(desc.offset, desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_PSLF_(CARD16 offset, CARD16 field) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PSLF %3d %3d", savedPC, offset, field);
	UNSPEC data = Pop();
	LONG_POINTER ptr = PopLong() + offset;
	CARD16* p = Store(ptr);
	// NO PAGE FAULT AFTER HERE
	*p = WriteField(*p, (CARD8)field, data);
	SP++; //Recover();
	SP++; //Recover();
}
// 0163  ASSIGN_MOP(z, PSLF)
void E_PSLF() {
	FieldDesc desc = {GetCodeWord()};
	E_PSLF_(desc.offset, desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 0164  ASSIGN_MOP(z, WLFS)
void E_WLFS() {
	FieldDesc desc = {Pop()};
	E_WLF_(desc.offset, desc.field);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_SLD_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  SLD %3d", savedPC, arg);
	POINTER ptr = LF + arg;
	CARD16* p0 = StoreMds(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : StoreMds(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = Pop();
	*p0 = Pop();
}
// 0165  ASSIGN_MOP(z, SLDB)
void E_SLDB() {
	E_SLD_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_SGD_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  SGD %3d", savedPC, arg);
	LONG_POINTER ptr = GF + arg;
	CARD16* p0 = Fetch(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Fetch(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	*p1 = Pop();
	*p0 = Pop();
}
// 0166  ASSIGN_MOP(z, SGDB)
void E_SGDB() {
	E_SGD_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_LLK_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  LLK %3d", savedPC, arg);
	CARD32 t = FetchLink(arg);
	// NO PAGE FAULT AFTER HERE
	PushLong(t);
}
// 0167  ASSIGN_MOP(z, LLKB)
void E_LLKB() {
	E_LLK_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RKI_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RKI %02X", savedPC, arg);
	LONG_POINTER ptr = FetchLink(arg);
	CARD16* p = Fetch(ptr);
	// NO PAGE FAULT AFTER HERE
	Push(*p);
}
// 0170  ASSIGN_MOP(z, RKIB)
void E_RKIB() {
	E_RKI_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_RKDI_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RKDI %02X", savedPC, arg);
	LONG_POINTER ptr = FetchLink(arg);
	CARD16* p0 = Fetch(ptr + 0);
	CARD16* p1 = isSamePage(ptr + 0, ptr + 1) ? (p0 + 1) : Fetch(ptr + 1);
	// NO PAGE FAULT AFTER HERE
	Push(*p0);
	Push(*p1);
}
// 0171  ASSIGN_MOP(z, RKDIB)
void E_RKDIB() {
	E_RKDI_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_LK_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  LK %02X", savedPC, arg);
	Recover();
	ShortControlLink link = Pop();
	CARD16* p = StoreMds(LF);
	// NO PAGE FAULT AFTER HERE
	*p = link - arg;
}
// 0172  ASSIGN_MOP(z, LKB)
void E_LKB() {
	E_LK_(GetCodeByte());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
__attribute__((always_inline)) static inline void E_SHIFT_(INT16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  SHIFT %3d", savedPC, arg);
	UNSPEC u = Pop();
	Push(Shift(u, arg));
}
// 0173  ASSIGN_MOP(z, SHIFT)
void E_SHIFT() {
	E_SHIFT_((INT16)Pop());
}
// 0174  ASSIGN_MOP(z, SHIFTSB)
void E_SHIFTSB() {
	E_SHIFT_(SignExtend(GetCodeByte()));
}
// 0175  //ASSIGN_MOP(z, MBP)
// 0176  //ASSIGN_MOP(z, RBP)
// 0177  //ASSIGN_MOP(z, WBP)
