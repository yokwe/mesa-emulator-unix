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
#include "Module.h"

#include <QtCore>

static QMap<LinkType, const char*> linkTypeMap {
	{LinkType::newProcedure, "NEWPROC"},
	{LinkType::oldProcedure, "OLDPROC"},
	{LinkType::frame,        "FRAME"},
	{LinkType::indirect,     "INDIRECT"},
};

const char* Trace::getLinkType(LinkType type) {
	if (linkTypeMap.contains(type)) {
		return linkTypeMap[type];
	} else {
		ERROR();
	}
}

static QMap<XferType, const char*> xferTypeMap {
	{XferType::return_,       "RET"},
	{XferType::call,          "CALL"},
	{XferType::localCall,     "LOCAL"},
	{XferType::port,          "PORT"},
	{XferType::xfer,          "XFER"},
	{XferType::trap,          "TRAP"},
	{XferType::processSwitch, "SWITCH"},
};

const char* Trace::getXferType(XferType type) {
	if (xferTypeMap.contains(type)) {
		return xferTypeMap[type];
	} else {
		ERROR();
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
		logger.info("%s", TO_CSTRING(string.trimmed()));
	} else {
		logger.info("%s  %s", TO_CSTRING(string), TO_CSTRING(extraMessae));
	}
}
static void message(const Trace::Context& context) {
	message(context, QStringLiteral(""));
}

QString Trace::Context::toString() const {
	const char* opcode = (callType == Trace::Context::CallType::XFER) ? "XFER" : "LFC";
	const char* xfer = Trace::getXferType(xferType);
	const char* free = freeFlag ? "*" : " ";
	const char* link = Trace::getLinkType(linkType);

	QString ret = QString::asprintf("%-4s %-6s %4X  FROM  %-30s %s%s  TO  %-40s %s  %-8s",
			opcode, xfer, oldPSB,
			TO_CSTRING(oldFunc.toString()), TO_CSTRING(oldFrame.toString()), free,
			TO_CSTRING(newFunc.toString()), TO_CSTRING(newFrame.toString()), link);

	return ret;
}

void Trace::Context::process_() {
	message(*this);
}


QSet<Trace::Func> Trace::Func::all;
static QMap<CARD16, QString> moduleNameMap;
//          gfi     moduleName
static QMap<QString, QMap<CARD16, QString>> funcNameMap;
//          moduleName    pc      funcName

void initMap() {
	Trace::Func::readLoadmapFile("data/map/Guam.loadmap");
	Trace::Func::readMapFile    ("data/map/GermGuam.map");
//	Trace::Func::readMapFile    ("data/map/BasicHeadsGuam.map");
}
QString Trace::Func::toString() const {
	if (moduleNameMap.isEmpty()) initMap();

	if (moduleNameMap.contains(gfi)) {
		const QString& moduleName = moduleNameMap[gfi];
		if (funcNameMap.contains(moduleName)) {
			const QMap<CARD16, QString>& map = funcNameMap[moduleName];
			//         pc      funcName
			if (map.contains(pc)) {
				const QString& funcName = map[pc];
				return QString("%1.%2").arg(moduleName).arg(funcName);
			} else {
				return QString("%1.%2").arg(moduleName).arg(pc, 4, 16, QChar('0'));

			}
		} else {
			return QString("%1.%2").arg(moduleName).arg(pc, 4, 16, QChar('0'));
		}
	} else {
		return QString::asprintf("%X-%04X", gfi, pc);
	}
}
void Trace::Func::addName(CARD16 gfi, const QString& moduleName) {
	if (moduleNameMap.contains(gfi)) {
		logger.fatal("Unexpeted");
		logger.fatal("  gfi = %4X", gfi);
		logger.fatal("  new = %s!", TO_CSTRING(moduleName));
		logger.fatal("  old = %s!", TO_CSTRING(moduleNameMap[gfi]));
		ERROR();
	} else {
		moduleNameMap[gfi] = moduleName;
//		logger.debug("addName %4X %s", gfi, TO_CSTRING(moduleName));
	}
}
void Trace::Func::addName(const QString& moduleName, const QString& funcName, CARD16 pc) {
	if (!funcNameMap.contains(moduleName)) {
		QMap<CARD16, QString> entry;
		funcNameMap[moduleName] = entry;
	}
	QMap<CARD16, QString>& map = funcNameMap[moduleName];
	//   pc      funcName
	if (map.contains(pc)) {
		logger.fatal("Unexpeted");
		logger.fatal("  moduleName = %s!", TO_CSTRING(moduleName));
		logger.fatal("  funcName   = %s!", TO_CSTRING(funcName));
		logger.fatal("  pc         = %d  0%oB", pc, pc);
		logger.fatal("  old        = %s!", TO_CSTRING(map[pc]));
		ERROR();
	} else {
		map[pc] = funcName;
//		logger.debug("addName %-30s  %-20s  %04X", TO_CSTRING(moduleName), TO_CSTRING(funcName), pc);
	}
}

void Trace::Func::readLoadmapFile(const QString& path) {
	QList<Module::LoadmapFile> list = Module::LoadmapFile::loadLoadmapFile(path);
	for(auto e: list) {
		addName(e.gfi, e.module);
	}
}

void Trace::Func::readMapFile(const QString& path) {
	QList<Module::MapFile> list = Module::MapFile::loadMapFile(path);
	for(auto e: list) {
		addName(e.module, e.proc, e.pc);
	}
}


