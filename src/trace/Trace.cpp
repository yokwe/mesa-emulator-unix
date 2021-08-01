/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
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


#include "../util/Util.h"
static log4cpp::Category& logger = Logger::getLogger("trace");

#include "../mesa/Memory.h"

#include "Trace.h"

#include <QtCore>

const char* Trace::getLinkType(LinkType type) {
	switch(type) {
	case LT_newProcedure:
		return "NEWPROC";
	case LT_oldProcedure:
		return "OLDPROC";
	case LT_frame:
		return "FRAME";
	case LT_indirect:
		return "INDIRECT";
	default:
		ERROR();
		return "ERROR";
	}
}

const char* Trace::getXferType(XferType type) {
	switch(type) {
	case XT_return:
		return "RET";
	case XT_call:
		return "CALL";
	case XT_localCall:
		return "LOCAL";
	case XT_port:
		return "PORT";
	case XT_xfer:
		return "XFER";
	case XT_trap:
		return "TRAP";
	case XT_processSwitch:
		return "SWITCH";
	default:
		ERROR();
		return "ERROR";
	}
}

static const char* ettName[256];
static QMap<CARD16, QSet<CARD16>> entryMap;
//          GFI          PC

static void initEttName() {
	char name[10];
	for(int i = 0; i < 256; i++) {
		sprintf(name, "ETT-%d", i);
		ettName[i] = strdup(name);
	}
}

#define CHECK_TRAP(name) if (controlLink == ReadDblMds(SD + OFFSET_SD(s##name))) return #name;

const char* Trace::getTrapName(ControlLink controlLink) {
	// init
	if (ettName[0] == 0) initEttName();

	CHECK_TRAP(Boot)
	CHECK_TRAP(BoundsTrap)
	CHECK_TRAP(BreakTrap)
	CHECK_TRAP(CodeTrap)
	CHECK_TRAP(ControlTrap)
	CHECK_TRAP(DivCheckTrap)
	CHECK_TRAP(DivZeroTrap)
	CHECK_TRAP(InterruptError)
	CHECK_TRAP(OpcodeTrap)
	CHECK_TRAP(PointerTrap)
	CHECK_TRAP(ProcessTrap)
	CHECK_TRAP(RescheduleError)
	CHECK_TRAP(StackError)
	CHECK_TRAP(UnboundTrap)
	CHECK_TRAP(XferTrap)
	CHECK_TRAP(HardwareError)

	for(int i = 0; i < 256; i++) {
		if (controlLink == ReadDblMds(ETT + OFFSET_ETT(i))) return ettName[i];
	}

	logger.warn("%s unknown controlLink %X", __FUNCTION__, controlLink);
	return "??";
}


class ProcessContext {
public:
	CARD16                       psb;
	QMap<CARD16, Trace::Context> active;
	QStack<Trace::Context>       callStack;

	ProcessContext(CARD16 psb_) : psb(psb_) {}

	ProcessContext() : psb(0) {}
	ProcessContext(const ProcessContext& that) : psb(that.psb), active(that.active), callStack(that.callStack) {}
	ProcessContext& operator= (const ProcessContext& that) {
		this->psb       = that.psb;
		this->active    = that.active;
		this->callStack = that.callStack;
		return *this;
	}

	void process(const Trace::Context& context);
};

void ProcessContext::process(const Trace::Context& context) {
	switch(context.xferType) {
	case XT_return:
		if (context.freeFlag) {
			Trace::Context lastContext = callStack.pop();
			if (lastContext.newLF == context.oldLF) {

			} else {
				logger.fatal("Unexpected");
				ERROR();
			}
		} else {
			logger.fatal("Unexpected");
			ERROR();
		}
		break;
	case XT_call:
		if (context.freeFlag) {
			logger.fatal("Unexpected");
			ERROR();
		} else {
			callStack.push(context);
		}
		break;
	case XT_port:
		if (context.freeFlag) {
			logger.fatal("Unexpected");
			ERROR();
		} else {
			//FIXME
		}
		break;
	case XT_xfer:
		if (context.freeFlag) {
			// FIXME
		} else {
			//FIXME
		}
		break;
	case XT_trap:
		if (context.freeFlag) {
			logger.fatal("Unexpected");
			ERROR();
		} else {
			//FIXME
		}
		break;
	case XT_processSwitch:
		if (context.freeFlag) {
			logger.fatal("Unexpected");
			ERROR();
		} else {
			//FIXME
		}
		break;
	case XT_localCall:
	default:
		logger.fatal("Unexpected");
		ERROR();
	}
}


static QMap<CARD16, ProcessContext> contextMap;
//          PSB          LF

void Trace::Context::process_() {
	if (!contextMap.contains(oldPSB)) {
		contextMap[oldPSB] = ProcessContext(oldPSB);
	}
	contextMap[oldPSB].process(*this);
}
void Trace::Context::message_() {
	const char* opcode = (callType == CT_XFER) ? "XFER" : "LFC";
	const char* xfer = getXferType(xferType);
	const char* link = getLinkType(linkType);
	const char* free = freeFlag ? "*" : " ";

	if (xferType == XT_trap) {
		logger.debug("%-4s %-6s %4X  FROM  %4X-%04X %04X%s  TO   %4X-%04X %04X  %-8s  %s",
			opcode, xfer, oldPSB, oldGFI, oldPC, oldLF, free, newGFI, newPC, newLF, link, getTrapName(dst));
	} else {
		logger.debug("%-4s %-6s %4X  FROM  %4X-%04X %04X%s  TO   %4X-%04X %04X  %-8s",
			opcode, xfer, oldPSB, oldGFI, oldPC, oldLF, free, newGFI, newPC, newLF, link);
	}
}
