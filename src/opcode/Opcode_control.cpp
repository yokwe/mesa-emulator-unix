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
// Opcode_control.cpp
//

#include "../util/Debug.h"
#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/Perf.h"

#include "../mesa/Constant.h"
#include "../mesa/Type.h"
#include "../mesa/Memory.h"
#include "../mesa/Function.h"
#include "../mesa/InterruptThread.h"

//#include "../trace/Trace.h"

#include "Opcode.h"


// 9.5.2 Trap Processing
// Trap: PROC[ptr: POINTER TO ControlLink]
static inline void Trap(POINTER ptr) {
	ControlLink handler = ReadDblMds(ptr);
	PC = savedPC;
	SP = savedSP;
	if (ValidContext()) *StoreMds(LO_OFFSET(LF, pc)) = PC;
	XFER(handler, LF, XferType::trap, 0);
}

// TrapZero: PROC[ptr: POINTER TO ControlLink]
static inline void TrapZero(POINTER ptr) {
	Trap(ptr);
	ERROR_Abort();
}

// TrapOne: PROC[ptr POINTER TO ControlLink, parameter: UNSPEC]
static inline void TrapOne(POINTER ptr, UNSPEC parameter) {
	Trap(ptr);
	*StoreMds(LF + 0) = parameter;
	ERROR_Abort();
}

// TrapTwo: PROC[ptr POINTER TO ControlLink, parameter: LONG UNSPEC]
static inline void TrapTwo(POINTER ptr, LONG_UNSPEC parameter) {
	Trap(ptr);
	*StoreMds(LF + 0) = LowHalf(parameter);
	*StoreMds(LF + 1) = HighHalf(parameter);
	ERROR_Abort();
}

// 9.5.1 Trap Routines
void BoundsTrap() {
	if (DEBUG_SHOW_BOUNDS_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sBoundsTrap));
}
void BreakTrap() {
	if (DEBUG_SHOW_BREAK_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sBreakTrap));
}
void CodeTrap(GFTHandle gfi) {
	PERF_COUNT(CodeTrap)
	if (DEBUG_SHOW_CODE_TRAP) logger.debug("%s %04X", __FUNCTION__, gfi);
	TrapOne(SD + OFFSET_SD(sCodeTrap), gfi);
}
void ControlTrap(ShortControlLink src) {
	if (DEBUG_SHOW_CONTROL_TRAP) logger.debug("%s %04X", __FUNCTION__, src);
	if (DEBUG_STOP_AT_CONTROL_TRAP) ERROR();
	TrapOne(SD + OFFSET_SD(sControlTrap), src);
}
void DivCheckTrap() {
	if (DEBUG_SHOW_DIV_CHECK_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sDivCheckTrap));
}
void DivZeroTrap() {
	if (DEBUG_SHOW_DIV_ZERO_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sDivZeroTrap));
}
void EscOpcodeTrap(BYTE opcode) {
	PERF_COUNT(EscOpcodeTrap)
	if (DEBUG_SHOW_ESC_OPCODE_TRAP) logger.debug("%s %03o", __FUNCTION__, opcode);
	if (DEBUG_STOP_AT_OPCODE_TRAP) ERROR();
	TrapOne(ETT + OFFSET_ETT(opcode), opcode);
}
void InterruptError() {
	if (DEBUG_SHOW_INTERRUPT_ERROR) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sInterruptError));
}
void OpcodeTrap(BYTE opcode) {
	PERF_COUNT(OpcodeTrap)
	if (DEBUG_SHOW_OPCODE_TRAP) logger.debug("%s %03o", __FUNCTION__, opcode);
	if (DEBUG_STOP_AT_OPCODE_TRAP) ERROR();
	TrapOne(SD + OFFSET_SD(sOpcodeTrap), opcode);
}
void PointerTrap() {
	if (DEBUG_SHOW_POINTER_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sPointerTrap));
}
void ProcessTrap() {
	if (DEBUG_SHOW_PROCESS_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sProcessTrap));
}
void RescheduleError() {
	if (DEBUG_SHOW_RESCHEDULE_TRAP) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sRescheduleError));
}
void StackError() {
	if (DEBUG_SHOW_STACK_ERROR) logger.debug("%s", __FUNCTION__);
	if (DEBUG_STOP_AT_STACK_ERROR) {
		logger.fatal("StackError  SP = %d savedSP = %d", SP, savedSP);
		logger.fatal("GFI = %04X  CB = %8X  PC = %4X  savedPC = %4X", GFI, CB, PC, savedPC);
		ERROR();
	}
	TrapZero(SD + OFFSET_SD(sStackError));
}
void UnboundTrap(ControlLink dst) {
	PERF_COUNT(UnboundTrap)
	if (DEBUG_SHOW_UNBOUND_TRAP) logger.debug("%s %08X", __FUNCTION__, dst);
	if (DEBUG_STOP_AT_UNBOUND_TRAP) ERROR();
	TrapTwo(SD + OFFSET_SD(sUnboundTrap), dst);
}
void HardwareError() {
	if (DEBUG_SHOW_HARDWARE_ERROR) logger.debug("%s", __FUNCTION__);
	TrapZero(SD + OFFSET_SD(sHardwareError));
}

// 9.5.5 Xfer Traps
// CheckForXferTraps: PROC[dst: ConrolLink, type: XferType]
static inline void CheckForXferTraps(ControlLink dst, XferType type) {
	if (Odd(XTS)) {
		GlobalWord word = {*Fetch(GO_OFFSET(GF, word))};
		if (word.trapxfers) {
			XTS = XTS >> 1;
			if (DEBUG_SHOW_XFER_TRAP) logger.debug("XferTrap %08X %d", dst, type);
			Trap(SD + OFFSET_SD(sXferTrap));
			*StoreMds(LF + 0) = LowHalf(dst);
			*StoreMds(LF + 1) = HighHalf(dst);
			*StoreMds(LF + 2) = (CARD16)type;
			ERROR_Abort();
		}
	} else {
		XTS = XTS >> 1;
	}
}


// 9.2.2 Frame Allocation Primitives

// Alloc: PROC[fsi: FSIndex] RETURNS[LocalFrameHandle]
static inline LocalFrameHandle Alloc(FSIndex fsi) {
	AVItem item;
	FSIndex slot = fsi;
	for(;;) {
		item.u = *FetchMds(AV + OFFSET_AV(slot));
		if (item.tag != (CARD16)AVItemType::indirect) break;
		if (FSIndex_SIZE <= item.data) ERROR();
		slot = item.data;
	}
	if (item.tag == (CARD16)AVItemType::empty) {
		FrameFault(fsi);
	}
	*StoreMds(AV + OFFSET_AV(slot)) = *FetchMds(AVLink(item.u));
	return AVFrame(item.u);
}

// Free: PROC[frame: LocalFrameHandle]
static inline void Free(LocalFrameHandle frame) {
	LocalWord word = {*FetchMds(LO_OFFSET(frame, word))};
	AVItem item = {*FetchMds(AV + OFFSET_AV(word.fsi))};
	*StoreMds(frame) = item.u;
	*StoreMds(AV + OFFSET_AV(word.fsi)) = frame;
}

// 9.3 Control Transfer Primitives

// XFER: PROC[dst: ControlLink, src: ShortControlLink, type: XferType, free: BOOLEAN = FALSE]
void XFER(ControlLink dst, ShortControlLink src, XferType type, int freeFlag = 0) {
	CARDINAL nGFI;
	CARDINAL nPC;
	CARDINAL nLF;
	int push = 0;
	ControlLink nDst = dst;

	// FIXME After GF, CB is updated, CodeTrap or FrameFault can be generated
	// FIXME In that case, GF and CB has wrong value before XFER is correctly executed.
	// FIXME We should use nGF and nCB and update GF and CB very end of this method

	if (type == XferType::trap && freeFlag) ERROR();
	while (ControlLinkType(nDst) == LinkType::indirect) {
		IndirectLink link = MakeIndirectLink(nDst);
		if (type == XferType::trap) ERROR();
		nDst = ReadDblMds(link);
		push = 1;
	}

	LinkType linkType = ControlLinkType(nDst);
	//Trace::Context context;
	//context.setXFER(dst, src, type, freeFlag, linkType, PSB, GFI, savedPC, Memory::MDS(), LF);

	switch (linkType) {
	case LinkType::oldProcedure : {
		ProcDesc proc = {MakeProcDesc(nDst)};
		CARD16 gf = proc.taggedGF & 0xfffc; // 177774
		if (gf == 0) UnboundTrap(dst);
		nGFI = *FetchMds(GO_OFFSET(gf, word)) & 0xfffc; // 177774
		if (nGFI == 0) UnboundTrap(dst);
		GF = ReadDbl(GFT_OFFSET(nGFI, globalFrame));
		if (GF != LengthenPointer(gf)) ERROR(); // Sanity check
		CB = ReadDbl(GFT_OFFSET(nGFI, codebase));
		if (CB & 1) {
			CodeTrap(nGFI);
		}
		nPC = proc.pc;
		if (nPC == 0) UnboundTrap(dst);
		BytePair word = { ReadCode(nPC / 2) };
		FSIndex fsi = ((nPC % 2) == 0) ? word.left : word.right;
		nLF = Alloc(fsi);
		nPC = nPC + 1;
		*StoreMds(LO_OFFSET(nLF, globallink)) = nGFI;
		*StoreMds(LO_OFFSET(nLF, returnlink)) = src;
	}
		break;
	case LinkType::frame : {
		FrameLink frame = {MakeFrameLink(nDst)};
		if (frame == 0) ControlTrap(src);
		nLF = frame;
		nGFI = *FetchMds(LO_OFFSET(nLF, globallink));
		if (nGFI == 0) UnboundTrap(dst);
		GF = ReadDbl(GFT_OFFSET(nGFI, globalFrame));
		CB = ReadDbl(GFT_OFFSET(nGFI, codebase));
		if (CB & 1) {
			CodeTrap(nGFI);
		}
		nPC = *FetchMds(LO_OFFSET(nLF, pc));
		if (nPC == 0) UnboundTrap(dst);
		if (type == XferType::trap) {
			*StoreMds(LO_OFFSET(nLF, returnlink)) = src;
			InterruptThread::disable();
		}
	}
		break;
	case LinkType::newProcedure : {
		NewProcDesc proc = {MakeNewProcDesc(nDst)};
		nGFI = proc.taggedGFI & 0xfffc; // 177774
		if (nGFI == 0) UnboundTrap(dst);
		GF = ReadDbl(GFT_OFFSET(nGFI, globalFrame));
		CB = ReadDbl(GFT_OFFSET(nGFI, codebase));
		if (CB & 1) {
			CodeTrap(nGFI);
		}
		nPC = proc.pc;
		if (nPC == 0) UnboundTrap(dst);
		BytePair word = { ReadCode(nPC / 2) };
		FSIndex fsi = ((nPC % 2) == 0) ? word.left : word.right;
		nLF = Alloc(fsi);
		nPC = nPC + 1;
		*StoreMds(LO_OFFSET(nLF, globallink)) = nGFI;
		*StoreMds(LO_OFFSET(nLF, returnlink)) = src;
	}
		break;
	default:
		ERROR();
		nPC = 0;
		nLF = 0;
		break;
	}

//	context.setContext(nGFI, nPC - 1, Memory::MDS(), nLF);
//	context.process();

	if (push) {
		Push((CARD16)dst);
		Push(src);
		Discard();
		Discard();
	}
	if (freeFlag)
		Free(LF);
	LF = nLF;
	GFI = nGFI;
	PC = nPC;
	CheckForXferTraps(dst, type);
}

// 9.4.2 External Function Calls
// Call: PROC[dst: ControlLink]
static inline void Call(ControlLink dst) {
	*StoreMds(LO_OFFSET(LF, pc)) = PC;
	XFER(dst, LF, XferType::call, 0);
}
///////////////////////////////////////////////////////////////////////

__attribute__((always_inline)) static inline void E_EFC_(CARD16 arg) {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  EFC %5d", savedPC, arg);
	Call(FetchLink(arg));
}
#define EFCn(n) \
void E_EFC##n () { \
	E_EFC_(n); \
}


// zEFC0 - 0337
EFCn(0)
// zEFC1 - 0340
EFCn(1)
// zEFC2 - 0341
EFCn(2)
// zEFC3 - 0342
EFCn(3)
// zEFC4 - 0343
EFCn(4)
// zEFC5 - 0344
EFCn(5)
// zEFC6 - 0345
EFCn(6)
// zEFC7 - 0346
EFCn(7)
// zEFC8 - 0347
EFCn(8)


// zEFC9 - 0350
EFCn(9)
// zEFC10 - 0351
EFCn(10)
// zEFC11 - 0352
EFCn(11)
// zEFC12 - 0353
EFCn(12)
// zEFCB - 0354
void E_EFCB() {
	E_EFC_(GetCodeByte());
}
// zLFC - 0355
void  E_LFC() {
//	Trace::Context context;
//	context.setLFC(0, 0, XferType::call, 0, LinkType::newProcedure, PSB, GFI, savedPC, Memory::MDS(), LF);

	CARDINAL nPC = GetCodeWord();
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  LFC %04X", savedPC, nPC);
	*StoreMds(LO_OFFSET(LF, pc)) = PC;
	if (nPC == 0) UnboundTrap(0);
	BytePair word = { ReadCode(nPC / 2) };
	FSIndex fsi = ((nPC % 2) == 0) ? word.left : word.right;
	CARD16 nLF = Alloc(fsi);
	nPC = nPC + 1;
	*StoreMds(LO_OFFSET(nLF, globallink)) = GFI;
	*StoreMds(LO_OFFSET(nLF, returnlink)) = LF;
	LF = nLF;
	PC = nPC;

//	context.setContext(GFI, nPC - 1, Memory::MDS(), nLF);
//	context.process();

	ProcDesc dst;
	dst.taggedGF = GFI | 1;
	dst.pc = PC - 1;
	CheckForXferTraps(dst.u, XferType::localCall);
}
// zSFC - 0356
void E_SFC() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  SFC", savedPC);
	ControlLink link = PopLong();
	Call(link);
}
// zRET - 0357
void E_RET() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  RET", savedPC);
	ControlLink dst = {*FetchMds(LO_OFFSET(LF, returnlink))};
	XFER(dst, 0, XferType::return_, 1);
}
// zKFCB - 0360
void E_KFCB() {
	SDIndex alpha = GetCodeByte();
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  KFCB %02X", savedPC, alpha);
	Call(ReadDblMds(SD + OFFSET_SD(alpha)));
}
// aAF - 012
void E_AF() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  AF", savedPC);
	FSIndex fsi = Pop();
	Push(Alloc(fsi));
}
// aFF - 013
void E_FF() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  FF", savedPC);
	LocalFrameHandle frame = Pop();
	Free(frame);
}
// aPI - 014
void E_PI() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PI", savedPC);
	Recover();
	Recover();
	ShortControlLink src = Pop();
	PortLink port = Pop();
	*StoreMds(port + OFFSET_PORT(inport)) = 0;
	if (src) *StoreMds(port + OFFSET_PORT(outport)) = src;
}
// aPO - 015
void E_PO() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  PO", savedPC);
	/* UNSPEC reserved = */
	Pop();
	PortLink port = Pop();
	*StoreMds(LO_OFFSET(LF, pc)) = PC;
	*StoreMds(port + OFFSET_PORT(inport)) = LF;
	XFER(ReadDblMds(port + OFFSET_PORT(outport)), port, XferType::port, 0);
}
// aPOR - 016
void E_POR() {
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  POR", savedPC);
	/* UNSPEC reserved = */
	Pop();
	PortLink port = Pop();
	*StoreMds(LO_OFFSET(LF, pc)) = PC;
	*StoreMds(port + OFFSET_PORT(inport)) = LF;
	XFER(ReadDblMds(port + OFFSET_PORT(outport)), port, XferType::port, 0);
}


// bXE - 041
void E_XE() {
	try {
		POINTER ptr = GetCodeByte();
		if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  XE %02X", savedPC, ptr);

		*StoreMds(LO_OFFSET(LF, pc)) = PC; // Store location of next instruction
		ControlLink      dst = ReadDblMds(LF + ptr + OFFSET(TransferDescriptor, dst));
		ShortControlLink src = *FetchMds (LF + ptr + OFFSET(TransferDescriptor, src));
		XFER(dst, src, XferType::xfer, 0);

		if (InterruptThread::getWDC() == 0) InterruptError();
		InterruptThread::enable();
	} catch(Abort &abort) {
		ERROR();
	}
}
// bXF - 042
void E_XF() {
	POINTER ptr = GetCodeByte();
	if (DEBUG_SHOW_OPCODE) logger.debug("TRACE %6o  XF %02X", savedPC, ptr);

	*StoreMds(LO_OFFSET(LF, pc)) = PC;  // Store location of next instruction
	ControlLink      dst = ReadDblMds(LF + ptr + OFFSET(TransferDescriptor, dst));
	ShortControlLink src = *FetchMds (LF + ptr + OFFSET(TransferDescriptor, src));
	XFER(dst, src, XferType::xfer, 1);
}
