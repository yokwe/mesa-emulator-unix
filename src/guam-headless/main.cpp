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
// main.cpp
//

#include "../util/Util.h"
#include <chrono>
#include <thread>
static const Logger logger(__FILE__);

#include "../util/Perf.h"

#include "../util/Setting.h"

#include "../util/GuiOp.h"

#include "../mesa/MesaProcessor.h"
#include "../mesa/Memory.h"

#include "../opcode/Interpreter.h"


int main(int /* argc */, char** /* argv */) {
	logger.info("START");

	GuiOp::setContext(new NullGuiOp);

	// sanity check
	// if (argc != 2) {
	// 	logger.error("Unexpected argc %d", argc);
	// 	ERROR();
	// }
	// std::string entryName = argv[1];
	std::string entryName = "GVWin";
	logger.info("entryName = %s", entryName);

	Setting setting = Setting::getInstance();
	Setting::Entry entry = setting.getEntry(entryName);

	CARD32  displayWidth     = entry.display.width;
	CARD32  displayHeight    = entry.display.height;

	std::string diskPath         = entry.file.disk;
	std::string germPath         = entry.file.germ;
	std::string bootPath         = entry.file.boot;
	std::string floppyPath       = entry.file.floppy;
	std::string bootSwitch       = entry.boot.switch_;
	std::string bootDevice       = entry.boot.device;
	std::string networkInterface = entry.network.interface;

	uint32_t vmBits           = entry.memory.vmbits;
	uint32_t rmBits           = entry.memory.rmbits;

	// stop at MP 8000
	ProcessorThread::stopAtMP( 915);
	ProcessorThread::stopAtMP(8000);

//	ProcessorThread::stopMessageUntilMP(930);

	MesaProcessor mesaProcessor;

	mesaProcessor.setDiskPath(diskPath);
	mesaProcessor.setGermPath(germPath);
	mesaProcessor.setBootPath(bootPath);
	mesaProcessor.setFloppyPath(floppyPath);
	mesaProcessor.setBootSwitch(bootSwitch);
	mesaProcessor.setBootDevice(bootDevice);

	mesaProcessor.setMemorySize(vmBits, rmBits);
	mesaProcessor.setDisplaySize(displayWidth, displayHeight);
	mesaProcessor.setNetworkInterfaceName(networkInterface);

	mesaProcessor.initialize();
	mesaProcessor.boot();

	Interpreter::stats();
	PERF_LOG();
	PageCache::stats();

	//extern void MonoBlt_MemoryCache_stats();
	//MonoBlt_MemoryCache_stats();

	//extern void MonoBlt_stats();
	//MonoBlt_stats();

	logger.info("elapsedTime = %lld msec", mesaProcessor.elapsedTime());

	logger.info("STOP");
	return 0;
}
