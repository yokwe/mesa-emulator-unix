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


static void message(const Trace::Context& context, const QString& extraMessae) {
	QString string = context.toString();

	if (extraMessae.isEmpty()) {
		logger.info("%s", string.trimmed().toLocal8Bit().constData());
	} else {
		logger.info("%s  %s", string.toLocal8Bit().constData(), extraMessae.toLocal8Bit().constData());
	}
}
static void message(const Trace::Context& context) {
	message(context, QStringLiteral(""));
}

QString Trace::Context::toString() const {
	const char* opcode = (callType == Trace::Context::CallType::CT_XFER) ? "XFER" : "LFC";
	const char* xfer = Trace::getXferType(xferType);
	const char* free = freeFlag ? "*" : " ";
	const char* link = Trace::getLinkType(linkType);

	QString ret = QString::asprintf("%-4s %-6s %4X  FROM  %9s %s%s  TO   %9s %s  %-8s",
			opcode, xfer, oldPSB,
			oldFunc.toString().toLocal8Bit().constData(), oldFrame.toString().toLocal8Bit().constData(), free,
			oldFunc.toString().toLocal8Bit().constData(), oldFrame.toString().toLocal8Bit().constData(), link);

	return ret;
}

void Trace::Context::process_() {
	message(*this);
}


QSet<Trace::Func> Trace::Func::all;

