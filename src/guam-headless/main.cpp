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

#include <thread>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/Perf.h"
#include "../util/Setting.h"
#include "../util/GuiOp.h"

#include "../mesa/guam.h"
#include "../mesa/processor.h"

#include "../opcode/opcode.h"

void startAndStop() {
	guam::initialize();
	guam::boot();
	guam::finalize();
	
	opcode::stats();
	PERF_LOG();
	memory::cache::stats();
	logger.info("elapsedTime = %lld msec", guam::getElapsedTime());
}

int main(int /* argc */, char** /* argv */) {
	logger.info("START");

	GuiOp::setContext(new NullGuiOp);

	// sanity check
	// if (argc != 2) {
	// 	logger.error("Unexpected argc %d", argc);
	// 	ERROR();
	// }
	// std::string entryName = argv[1];
	auto entryName = "GVWin";
	logger.info("entryName = %s", entryName);

	auto setting = Setting::getInstance();
	auto entry   = setting.getEntry(entryName);

	auto displayWidth     = entry.display.width;
	auto displayHeight    = entry.display.height;

	auto diskPath         = entry.file.disk;
	auto germPath         = entry.file.germ;
	auto bootPath         = entry.file.boot;
	auto floppyPath       = entry.file.floppy;
	auto bootSwitch       = entry.boot.switch_;
	auto bootDevice       = entry.boot.device;
	auto networkInterface = entry.network.interface;

	auto vmBits           = entry.memory.vmbits;
	auto rmBits           = entry.memory.rmbits;

	guam::setDiskPath(diskPath);
	guam::setGermPath(germPath);
	guam::setBootPath(bootPath);
	guam::setFloppyPath(floppyPath);
	guam::setBootSwitch(bootSwitch);
	guam::setBootDevice(bootDevice);

	guam::setMemorySize(vmBits, rmBits);
	guam::setDisplaySize(displayWidth, displayHeight);
	guam::setNetworkInterfaceName(networkInterface);

	// stop at MP 8000
	processor::stopAtMP( 915);
	processor::stopAtMP(8000);

	logger.info("thread start");
	auto thread = std::thread(startAndStop);
	logger.info("thread joinning");
	thread.join();
	logger.info("thread joined");

	// {
	// 	guam::initialize();
	// 	guam::boot();
	// 	guam::finalize();
		
	// 	opcode::stats();
	// 	PERF_LOG();
	// 	memory::cache::stats();
	// 	logger.info("elapsedTime = %lld msec", guam::getElapsedTime());
	// }


	//extern void MonoBlt_MemoryCache_stats();
	//MonoBlt_MemoryCache_stats();

	//extern void MonoBlt_stats();
	//MonoBlt_stats();

	logger.info("STOP");
	return 0;
}
