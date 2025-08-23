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


#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Memory.h"

#include "Trace.h"
#include "Module.h"


static std::map<LinkType, const char*> linkTypeMap {
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

static std::map<XferType, const char*> xferTypeMap {
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
static std::map<CARD16, std::set<CARD16>> entryMap;
//          GFI          PC

static void initEttName() {
	char name[10];
	for(int i = 0; i < 256; i++) {
		snprintf(name, sizeof(name), "ETT-%d", i);
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


static void message(const Trace::Context& context, const std::string& extraMessage) {
	std::string string = context.toString();

	if (extraMessage.isEmpty()) {
		logger.info("%s", string.trimmed());
	} else {
		logger.info("%s  %s", string, extraMessage);
	}
}
static void message(const Trace::Context& context) {
	message(context, std::stringLiteral(""));
}

std::string Trace::Context::toString() const {
	const char* opcode = (callType == Trace::Context::CallType::XFER) ? "XFER" : "LFC";
	const char* xfer = Trace::getXferType(xferType);
	const char* free = freeFlag ? "*" : " ";
	const char* link = Trace::getLinkType(linkType);

	std::string ret = std::string::asprintf("%-4s %-6s %4X  FROM  %-30s %s%s  TO  %-40s %s  %-8s",
			opcode, xfer, oldPSB,
			qPrintable(oldFunc.toString()), qPrintable(oldFrame.toString()), free,
			qPrintable(newFunc.toString()), qPrintable(newFrame.toString()), link);

	return ret;
}

void Trace::Context::process_() {
	message(*this);
}


std::set<Trace::Func> Trace::Func::all;
static std::map<CARD16, std::string> moduleNameMap;
//          gfi     moduleName
static std::map<std::string, std::map<CARD16, std::string>> funcNameMap;
//          moduleName    pc      funcName

void initMap() {
	Trace::Func::readLoadmapFile("data/map/Guam.loadmap");
	Trace::Func::readMapFile    ("data/map/GermGuam.map");
//	Trace::Func::readMapFile    ("data/map/BasicHeadsGuam.map");
}
std::string Trace::Func::toString() const {
	if (moduleNameMap.isEmpty()) initMap();

	if (moduleNameMap.contains(gfi)) {
		const std::string& moduleName = moduleNameMap[gfi];
		if (funcNameMap.contains(moduleName)) {
			const std::map<CARD16, std::string>& map = funcNameMap[moduleName];
			//         pc      funcName
			if (map.contains(pc)) {
				const std::string& funcName = map[pc];
				return std::string("%1.%2").arg(moduleName).arg(funcName);
			} else {
				return std::string("%1.%2").arg(moduleName).arg(pc, 4, 16, QChar('0'));

			}
		} else {
			return std::string("%1.%2").arg(moduleName).arg(pc, 4, 16, QChar('0'));
		}
	} else {
		return std::string::asprintf("%X-%04X", gfi, pc);
	}
}
void Trace::Func::addName(CARD16 gfi, const std::string& moduleName) {
	if (moduleNameMap.contains(gfi)) {
		logger.fatal("Unexpeted");
		logger.fatal("  gfi = %4X", gfi);
		logger.fatal("  new = %s!", moduleName);
		logger.fatal("  old = %s!", moduleNameMap[gfi]);
		ERROR();
	} else {
		moduleNameMap[gfi] = moduleName;
//		logger.debug("addName %4X %s", gfi, moduleName);
	}
}
void Trace::Func::addName(const std::string& moduleName, const std::string& funcName, CARD16 pc) {
	if (!funcNameMap.contains(moduleName)) {
		std::map<CARD16, std::string> entry;
		funcNameMap[moduleName] = entry;
	}
	std::map<CARD16, std::string>& map = funcNameMap[moduleName];
	//   pc      funcName
	if (map.contains(pc)) {
		logger.fatal("Unexpeted");
		logger.fatal("  moduleName = %s!", moduleName);
		logger.fatal("  funcName   = %s!", funcName);
		logger.fatal("  pc         = %d  0%oB", pc, pc);
		logger.fatal("  old        = %s!", map[pc]);
		ERROR();
	} else {
		map[pc] = funcName;
//		logger.debug("addName %-30s  %-20s  %04X", moduleName, funcName, pc);
	}
}

void Trace::Func::readLoadmapFile(const std::string& path) {
	std::vector<Module::LoadmapFile> list = Module::LoadmapFile::loadLoadmapFile(path);
	for(auto e: list) {
		addName(e.gfi, e.module);
	}
}

void Trace::Func::readMapFile(const std::string& path) {
	std::vector<Module::MapFile> list = Module::MapFile::loadMapFile(path);
	for(auto e: list) {
		addName(e.module, e.proc, e.pc);
	}
}


