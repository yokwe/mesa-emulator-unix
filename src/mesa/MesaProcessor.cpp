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


//
// MesaProcessor.cpp
//

#include "../util/Util.h"
static const util::Logger logger(__FILE__);


#include "../util/Debug.h"

#include "../opcode/Interpreter.h"

#include "../agent/StreamBoot.h"
#include "../agent/StreamCopyPaste.h"
#include "../agent/StreamPCFA.h"
#include "../agent/StreamTCP.h"
#include "../agent/StreamWWC.h"

#include "MesaProcessor.h"


void MesaProcessor::initialize() {
	setSignalHandler();

	QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD);

	logger.info("vmBits = %2d  rmBits = %2d", vmBits, rmBits);
	Memory::initialize(vmBits, rmBits, Agent::ioRegionPage);
	Interpreter::initialize();

	// Reserve real memory for display
	Memory::reserveDisplayPage(displayWidth, displayHeight);

	// AgentDisk use diskFile
	for(int i = 1; i <= 999; i++) {
		std::string path;
		if (diskPath.contains("%1", Qt::CaseSensitivity::CaseSensitive)) {
			path = diskPath.arg(i, 3, 10, QLatin1Char('0'));
		} else {
			path = diskPath;
		}
		if (!QFile::exists(path)) break;

		logger.info("Disk  %s", path.toStdString());

		DiskFile* diskFile = new DiskFile;
		diskFile->attach(path);
		diskFileList.append(diskFile);
		disk.addDiskFile(diskFile);

		// Handle Dawn.dsk properly
		if (path == diskPath) break;
	}

	logger.info("Floppy %s", floppyPath.toStdString());
	floppyFile.attach(floppyPath);
	floppy.addDiskFile(&floppyFile);

	// AgentNetwork use networkPacket
	logger.info("networkInterfaceName = %s", networkInterfaceName.toStdString());
	networkPacket.attach(networkInterfaceName);
	network.setNetworkPacket(&networkPacket);

	// AgentProcessor::Initialize use PID[]
	PID[0] = 0;
	networkPacket.getAddress(PID[1], PID[2], PID[3]);
	processor.setProcessorID(PID[1], PID[2], PID[3]);

	// set display width and height
	display.setDisplayWidth(displayWidth);
	display.setDisplayHeight(displayHeight);

	// Initialization of Agent
	Agent::InitializeAgent();
	logger.info("Agent FCB  %04X %04X", Agent::ioRegionPage * PageSize, Agent::getIORegion());

	logger.info("Boot  %s", bootPath.toStdString());

	// Initialization of Stream handler
	AgentStream* agentStream = (AgentStream*)Agent::getAgent((int)GuamInputOutput::AgentDeviceIndex::stream);
	// 110 Boot
	agentStream->addStream(new StreamBoot(bootPath));
	// 101 CopyPaste
	agentStream->addStream(new StreamCopyPaste);
	//   1 PCFA
	agentStream->addStream(new StreamPCFA);
	//  21 TCP
	agentStream->addStream(new StreamTCP);
	// 108 WWC
	agentStream->addStream(new StreamWWC);

	// load germ file into vm
	loadGerm(germPath);

	// set boot request
	{
		Boot::Request* request = (Boot::Request*)Memory::getAddress(SD + OFFSET_SD(SDDefs::sFirstGermRequest));

		// clear boot request
		::memset(request, 0, sizeof(*request));

		logger.info("bootDevice %s", bootDevice.toStdString());
		if (bootDevice == "DISK") {
			setBootRequestPV(request);
		} else if (bootDevice == "ETHER") {
			setBootRequestEther(request);
		} else if (bootDevice == "STREAM") {
			setBootRequestStream(request);
		} else {
			logger.fatal("Unknown bootDevice");
			ERROR()
		}

		logger.info("bootSwitch = %s", bootSwitch.toStdString());
		setSwitches(request->switches, TO_CSTRING(bootSwitch));
	}

	// setAutoDelete(false) for interruptThread, timerThread and processorThread.
	interruptThread.setAutoDelete(false);
	timerThread.setAutoDelete(false);
	processorThread.setAutoDelete(false);
	//
	setRunning(0);
}

void MesaProcessor::boot() {
	setRunning(1);
	//
	logger.info("MesaProcessor::boot START");
	QThreadPool::globalInstance()->start(&interruptThread);
	QThreadPool::globalInstance()->start(&timerThread);
	QThreadPool::globalInstance()->start(&network.receiveThread);
	QThreadPool::globalInstance()->start(&network.transmitThread);
	QThreadPool::globalInstance()->start(&disk.ioThread);
	QThreadPool::globalInstance()->start(&processorThread);
	logger.info("MesaProcessor::boot STOP");
}

void MesaProcessor::stop() {
	logger.info("MesaProcessor::stop START");
	processorThread.stop();
	logger.info("MesaProcessor::stop STOP");
}

void MesaProcessor::wait() {
	logger.info("MesaProcessor::wait START");
	logger.info("wait for all threads");
	QThreadPool::globalInstance()->waitForDone();
	logger.info("MesaProcessor::wait STOP");
	//
	setRunning(0);
	// Properly detach DiskFile
	for(DiskFile* diskFile: diskFileList) {
		diskFile->detach();
	}
	floppyFile.detach();
}

void MesaProcessor::loadGerm(std::string& path) {
	logger.info("germ  path    = %s", path.toStdString());

	CARD32 mapSize = 0;
	DiskFile::Page* map = (DiskFile::Page*)Util::mapFile(path, mapSize);
	CARD32 mapPageSize = mapSize / sizeof(DiskFile::Page);
	logger.info("germ  size = %d  %04X", mapPageSize, mapPageSize);

	// first page goes to mGFT
	CARD16 *p = Memory::getAddress(mGFT);
	for(CARD32 i = 0; i < mapPageSize; i++) {
		if (i == (int)GermOpsImpl::pageEndGermVM) {
			logger.fatal("i == pageEndGermVM");
			exit(1);
		}

		for(int j = 0; j < PageSize; j++) {
			p[j] = qFromBigEndian(map[i].word[j]);
		}

		p = Memory::getAddress(((i + 1) * PageSize));
	}
	Util::unmapFile(map);
}

static void setSwitch(System::Switches& switches, unsigned char c) {
	logger.info("setSwitch %c %3o", c, c);
	int high = (c >> 4) & 0x0f;
	int low = c & 0x0f;

	switches.word[high] |= (CARD16)(1U << (15 - low));
}
void MesaProcessor::setSwitches(System::Switches& switches, const char *string) {
	int len = ::strlen(string);
	for(int i = 0; i < len; i++) {
		// decode \[0-3][0-7][0-7] character sequence
		if (string[i] == '\\') {
			int n = 0;
			char c1 = string[i + 1];
			char c2 = string[i + 2];
			char c3 = string[i + 3];
			i += 3;

			if ('0' <= c1 && c1 <= '3') {
				n = (c1 - '0') * 64;
			} else {
				logger.fatal("c1 = %c", c1);
				ERROR();
			}
			if ('0' <= c2 && c2 <= '7') {
				n += (c2 - '0') * 8;
			} else {
				logger.fatal("c2 = %c", c1);
				ERROR();
			}
			if ('0' <= c3 && c3 <= '7') {
				n += (c3 - '0');
			} else {
				logger.fatal("c3 = %c", c1);
				ERROR();
			}
			setSwitch(switches, (char)(n & 0xff));
		} else {
			setSwitch(switches, string[i]);
		}
	}
}
void MesaProcessor::setBootRequestPV(Boot::Request* request, CARD16 deviceOrdinal) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_bootPhysicalVolume;
	request->location.deviceType    = Device::T_anyPilotDisk;
	request->location.deviceOrdinal = deviceOrdinal;
}
void MesaProcessor::setBootRequestEther(Boot::Request* request, CARD16 deviceOrdinal) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_inLoad;
	request->location.deviceType    = Device::T_ethernet;
	request->location.deviceOrdinal = deviceOrdinal;
	request->location.ethernetRequest.bfn.word[0]            = 0x0000; // Unique number assigned to each boot file based on 48 bit host number
	request->location.ethernetRequest.bfn.word[1]            = 0xaa00;
	request->location.ethernetRequest.bfn.word[2]            = 0x0e60;
	request->location.ethernetRequest.address.net.word[0]    = 0x0000; // unknown
	request->location.ethernetRequest.address.net.word[1]    = 0x0000;
	request->location.ethernetRequest.address.host.word[0]   = 0xffff; // broadcast
	request->location.ethernetRequest.address.host.word[1]   = 0xffff;
	request->location.ethernetRequest.address.host.word[2]   = 0xffff;
	request->location.ethernetRequest.address.socket.word[0] = 0x000a; // boot socket
}

void MesaProcessor::setBootRequestStream(Boot::Request* request) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_inLoad;
	request->location.deviceType    = Device::T_simpleDataStream;
	request->location.deviceOrdinal = 0;
}

void MesaProcessor::setRunning(int newValue) {
	running.storeRelease(newValue);
}

int MesaProcessor::getRunning() {
	return running.loadAcquire();
}

