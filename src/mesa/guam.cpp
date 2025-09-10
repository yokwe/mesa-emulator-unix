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
// guam.cpp
//

#include <csignal>
#include <thread>

#include "../util/Util.h"
#include "Variable.h"
static const Logger logger(__FILE__);

#include "MesaBasic.h"
#include "memory.h"
#include "Pilot.h"

#include "interrupt.h"
#include "timer.h"
#include "processor.h"

#include "../agent/Agent.h"
#include "../agent/AgentDisk.h"
#include "../agent/AgentFloppy.h"
#include "../agent/AgentNetwork.h"
#include "../agent/AgentKeyboard.h"
#include "../agent/AgentBeep.h"
#include "../agent/AgentMouse.h"
#include "../agent/AgentProcessor.h"
#include "../agent/AgentStream.h"
#include "../agent/AgentDisplay.h"

#include "../agent/DiskFile.h"
#include "../agent/NetworkPacket.h"

#include "../agent/StreamBoot.h"
#include "../agent/StreamCopyPaste.h"
#include "../agent/StreamPCFA.h"
#include "../agent/StreamTCP.h"
#include "../agent/StreamWWC.h"

#include "../opcode/opcode.h"

#include "guam.h"

namespace guam {

std::string    diskPath;
std::string    germPath;
std::string    bootPath;
std::string    floppyPath;
std::string    bootSwitch;
std::string    bootDevice;
int            vmBits;
int            rmBits;
CARD16         displayWidth;
CARD16         displayHeight;
std::string    networkInterfaceName;

DiskFile         diskFile;
DiskFile         floppyFile;
NetworkPacket    networkPacket;

// agent
AgentDisk      disk;
AgentFloppy    floppy;
AgentNetwork   network;
//	AgentParallel  parallel;
AgentKeyboard  keyboard;
AgentBeep      beep;
AgentMouse     mouse;
AgentProcessor agentProcessor;
AgentStream    stream;
//	AgentSerial    serial;
//	AgentTTY       tty;
AgentDisplay   display;
//	AgentReserved3 reserved3;

int64_t timeStart;
int64_t timeStop;


void setDiskPath(const std::string& diskPath_) {
    diskPath = diskPath_;
}
void setGermPath(const std::string& germPath_) {
    germPath = germPath_;
}
void setBootPath(const std::string& bootPath_) {
    bootPath = bootPath_;
}
void setFloppyPath(const std::string& floppyPath_) {
    floppyPath = floppyPath_;
}
void setBootSwitch(const std::string& bootSwitch_) {
    bootSwitch = bootSwitch_;
}
void setBootDevice(const std::string& bootDevice_) {
    bootDevice = bootDevice_;
}
void setMemorySize(int vmBits_, int rmBits_) {
    vmBits = vmBits_;
    rmBits = rmBits_;
}
void setDisplaySize(CARD16 displayWidth_, CARD16 displayHeight_) {
    displayWidth  = displayWidth_;
    displayHeight = displayHeight_;
}
void setNetworkInterfaceName(const std::string& networkInterfaceName_) {
    networkInterfaceName = networkInterfaceName_;
}

void loadGerm(std::string& path) {
	logger.info("germ  path    = %s", path);

	CARD32 mapSize = 0;
	DiskFile::Page* map = (DiskFile::Page*)Util::mapFile(path, mapSize);
	CARD32 mapPageSize = mapSize / sizeof(DiskFile::Page);
	logger.info("germ  size = %d  %04X", mapPageSize, mapPageSize);

	// first page goes to mGFT
	CARD16 *p = memory::peek(mGFT);
	for(CARD32 i = 0; i < mapPageSize; i++) {
		if (i == (int)GermOpsImpl::pageEndGermVM) {
			logger.fatal("i == pageEndGermVM");
			exit(1);
		}

		Util::byteswap(map[i].word, p, PageSize);

		p = memory::peek(((i + 1) * PageSize));
	}
	Util::unmapFile(map);
}

void initialize() {
	setSignalHandler(SIGINT);
	setSignalHandler(SIGTERM);
	setSignalHandler(SIGHUP);
	setSignalHandler(SIGSEGV);

	logger.info("vmBits = %2d  rmBits = %2d", vmBits, rmBits);
	memory::initialize(vmBits, rmBits, agent::ioRegionPage);
	opcode::initialize();
	variable::initialize();
	agent::initialize();

	// Reserve real memory for display
	memory::reserveDisplayPage(displayWidth, displayHeight);

	//
	// Initialize Agent
	//
	// AgentDisk
	logger.info("Disk  %s", diskPath);
	diskFile.attach(diskPath);
	disk.addDiskFile(&diskFile);
	// AgentFloppy
	logger.info("Floppy %s", floppyPath);
	floppyFile.attach(floppyPath);
	floppy.addDiskFile(&floppyFile);
	// AgentNetwork use networkPacket
	logger.info("networkInterfaceName = %s", networkInterfaceName);
	networkPacket.attach(networkInterfaceName);
	network.setNetworkPacket(&networkPacket);
	// AgentProcessor::Initialize use PID[]
	// get PID from network adapter
	networkPacket.getAddress(PID[1], PID[2], PID[3]);
	logger.info("PID = %04X-%04X-%04X", PID[1], PID[2], PID[3]);
	// set PID to AgentProcessor
	agentProcessor.setProcessorID(PID[1], PID[2], PID[3]);
	// AgentDisplay
	logger.info("displayWidth  = %4d", displayWidth);
	logger.info("displayHeight = %4d", displayHeight);
	display.setDisplayWidth(displayWidth);
	display.setDisplayHeight(displayHeight);
	// Stream::Boot
	logger.info("bootPath = %s", bootPath);

	// Enable Agents
	disk.Enable();
	floppy.Enable();
	network.Enable();
//	AgentParallel  parallel;
	keyboard.Enable();
	beep.Enable();
	mouse.Enable();
	agentProcessor.Enable();
	stream.Enable();
//	AgentSerial    serial;
//	AgentTTY       tty;
	display.Enable();
//	AgentReserved3 reserved3;
	logger.info("Agent FCB  %04X %04X", agent::ioRegionPage * PageSize, agent::getIORegion());

	// Initialization of Stream handler
	AgentStream* agentStream = (AgentStream*)agent::getAgent((int)GuamInputOutput::AgentDeviceIndex::stream);
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
		Boot::Request* request = (Boot::Request*)memory::peek(SD + OFFSET_SD(SDDefs::sFirstGermRequest));

		// clear boot request
		::memset(request, 0, sizeof(*request));

		logger.info("bootDevice %s", bootDevice);
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

		logger.info("bootSwitch = %s", bootSwitch);
		setSwitches(request->switches, bootSwitch.c_str());
	}

	timeStart = timeStop = 0;
}

class ThreadControl {
public:
	std::string name;
	std::function<void()> function;
	std::thread thread;

	ThreadControl(const char* name_, std::function<void()> function_) : name(name_), function(function_) {}

	void start() {
		logger.info("thread start   %s", name);
		thread = std::thread(function);
	}
	void stop() {
		logger.info("thread joining %s", name);
		thread.join();
		logger.info("thread joined  %s", name);
	}
};

void boot() {
	logger.info("boot START");

	std::function<void()> f1 = std::function<void()>(interrupt::run);
	std::function<void()> f2 = std::function<void()>(timer::run);
	std::function<void()> f3 = std::bind(&AgentNetwork::ReceiveThread::run, &network.receiveThread);
	std::function<void()> f4 = std::bind(&AgentNetwork::TransmitThread::run, &network.transmitThread);
	std::function<void()> f5 = std::bind(&AgentDisk::IOThread::run, &disk.ioThread);
	std::function<void()> f6 = std::function<void()>(processor::run);

	ThreadControl t1("interrupt", f1);
	ThreadControl t2("timer", f2);
	ThreadControl t3("receive", f3);
	ThreadControl t4("transmit", f4);
	ThreadControl t5("disk", f5);
	ThreadControl t6("processor", f6);

	timeStart = Util::getMilliSecondsFromEpoch();
	t1.start();
	t2.start();
	t3.start();
	t4.start();
	t5.start();
	t6.start();

	std::this_thread::yield();
	
	t1.stop();
	t2.stop();
	t3.stop();
	t4.stop();
	t5.stop();
	t6.stop();
	timeStop = Util::getMilliSecondsFromEpoch();

	logger.info("boot STOP");

	// Properly detach DiskFile
	diskFile.detach();
	floppyFile.detach();
}

static void setSwitch(System::Switches& switches, unsigned char c) {
	logger.info("setSwitch %c %3o", c, c);
	int high = (c >> 4) & 0x0f;
	int low = c & 0x0f;

	switches.word[high] |= (CARD16)(1U << (15 - low));
}
void setSwitches(System::Switches& switches, const char *string) {
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
void setBootRequestPV(Boot::Request* request, CARD16 deviceOrdinal) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_bootPhysicalVolume;
	request->location.deviceType    = Device::T_anyPilotDisk;
	request->location.deviceOrdinal = deviceOrdinal;
}
void setBootRequestEther(Boot::Request* request, CARD16 deviceOrdinal) {
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

void setBootRequestStream(Boot::Request* request) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_inLoad;
	request->location.deviceType    = Device::T_simpleDataStream;
	request->location.deviceOrdinal = 0;
}

int64_t elapsedTime() {
    return timeStop - timeStart;
}

}