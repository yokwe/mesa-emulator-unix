/*
Copyright (c) 2014, Yasuhiro Hasegawa
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/


//
// main.cpp
//

#include "../util/Util.h"
static log4cpp::Category& logger = Logger::getLogger("main");

#include "../util/Perf.h"

#include "../util/Setting.h"

#include "../util/GuiOp.h"

#include "../mesa/MesaProcessor.h"
#include "../mesa/Memory.h"

#include "../opcode/Interpreter.h"

#include <QtCore>

int main(int argc, char** argv) {
	logger.info("START");

	GuiOp::setContext(new NullGuiOp);

	// sanity check
	if (argc != 2) {
		logger.error("Unexpected argc %d", argc);
		ERROR();
	}
	QString entryName = argv[1];
	logger.info("Section = %s", entryName.toLocal8Bit().constData());

	Setting::Entry entry = Setting::getInstance(entryName);

	CARD32  displayWidth     = entry.display.width;
	CARD32  displayHeight    = entry.display.height;

	QString diskPath         = entry.file.disk;
	QString germPath         = entry.file.germ;
	QString bootPath         = entry.file.boot;
	QString floppyPath       = entry.file.floppy;
	QString bootSwitch       = entry.boot.switch_;
	QString bootDevice       = entry.boot.device;
	QString networkInterface = entry.network.interface;

	quint32 vmBits           = entry.memory.vmbits;
	quint32 rmBits           = entry.memory.rmbits;

	// stop at MP 8000
	perf_stop_at_mp_8000 = 1;

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

	// measure elapsed time between boot and MP8000
	QElapsedTimer elapsedTimer;
	elapsedTimer.start();
	mesaProcessor.boot();
	mesaProcessor.wait();
	quint64 elapsedTime = elapsedTimer.nsecsElapsed();

	Interpreter::stats();
	PERF_LOG();
	PageCache::stats();
	CodeCache::stats();
	LFCache::stats();

	//extern void MonoBlt_MemoryCache_stats();
	//MonoBlt_MemoryCache_stats();

	//extern void MonoBlt_stats();
	//MonoBlt_stats();

	logger.info("elapsedTime = %lld msec", elapsedTime / (1000 * 1000)); // display as milliseconds

	logger.info("STOP");
	return 0;
}
