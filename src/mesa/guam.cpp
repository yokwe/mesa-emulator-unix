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

#include <array>
#include <csignal>
#include <cstring>
#include <thread>
#include <map>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "MesaBasic.h"
#include "memory.h"
#include "Pilot.h"

#include "interrupt_thread.h"
#include "timer_thread.h"
#include "processor_thread.h"
#include "keymap.h"
#include "display.h"

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

int64_t        elapsedTime;
Config         config;

DiskFile       diskFile;
DiskFile       floppyFile;
NetworkPacket  networkPacket;

// agent
AgentDisk      disk;
AgentFloppy    floppy;
AgentNetwork   network;
//	AgentParallel  parallel;
AgentKeyboard  keyboard;
AgentBeep      beep;
AgentMouse     mouse;
AgentProcessor processor;
AgentStream    stream;
//	AgentSerial    serial;
//	AgentTTY       tty;
AgentDisplay   display;
//	AgentReserved3 reserved3;

display::DisplayContext displayContext;

void setConfig(const Config& config_) {
	config = config_;
}
void getConfig(Config& config_) {
	config_ = config;
}
static void loadGerm(std::string& path) {
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
static void setSwitch(System::Switches& switches, unsigned char c) {
	logger.info("setSwitch %c %3o", c, c);
	int high = (c >> 4) & 0x0f;
	int low = c & 0x0f;

	switches.word[high] |= (CARD16)(1U << (15 - low));
}
static void setSwitches(System::Switches& switches, const char *string) {
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
static void setBootRequestPV(Boot::Request* request, CARD16 deviceOrdinal = 0) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_bootPhysicalVolume;
	request->location.deviceType    = Device::T_anyPilotDisk;
	request->location.deviceOrdinal = deviceOrdinal;
}
static void setBootRequestEther(Boot::Request* request, CARD16 deviceOrdinal = 0) {
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
static void setBootRequestStream(Boot::Request* request) {
	request->requestBasicVersion    = Boot::currentRequestBasicVersion;
	request->action                 = Boot::A_inLoad;
	request->location.deviceType    = Device::T_simpleDataStream;
	request->location.deviceOrdinal = 0;
}

static std::map<std::string, CARD16> displayTypeMap = {
	{"monochrome", DisplayIOFaceGuam::T_monochrome},
	{"fourBitPlaneColor", DisplayIOFaceGuam::T_fourBitPlaneColor},
	{"byteColor", DisplayIOFaceGuam::T_byteColor},
};
static void initialize() {
	setSignalHandler(SIGINT);
	setSignalHandler(SIGTERM);
	setSignalHandler(SIGHUP);
	setSignalHandler(SIGSEGV);

	logger.info("diskFilePath      %s", config.diskFilePath);
	logger.info("germFilePath      %s", config.germFilePath);
	logger.info("bootFilePath      %s", config.bootFilePath);
	logger.info("floppyFilePath    %s", config.floppyFilePath);
	logger.info("networkInterface  %s", config.networkInterface);
	logger.info("bootSwitch        %s", config.bootSwitch);
	logger.info("bootDevice        %s", config.bootDevice);
	logger.info("displayType       %s", config.displayType);

	logger.info("displayWidth   %4d", config.displayWidth);
	logger.info("displayHeight  %4d", config.displayHeight);
	logger.info("vmBits         %4d", config.vmBits);
	logger.info("rmBits         %4d", config.rmBits);

	// start initialize
	elapsedTime = 0;
	memory::initialize(config.vmBits, config.rmBits, agent::ioRegionPage);
	opcode::initialize();
	variable::initialize();
	agent::initialize();

	elapsedTime = 0;

	//
	// Setup Agents
	//
	// AgentDisplay
	{
		auto displayType = displayTypeMap.at(config.displayType);
		displayContext = display::DisplayContext::getInstance(displayType, config.displayWidth, config.displayHeight);
		logger.info("displayContext  %s  %d x %d   %d pages", config.displayType, displayContext.width, displayContext.height, displayContext.pagesForBitmap);
		// Reserve real memory for display
		memory::reserveDisplayPage(displayContext.pagesForBitmap);
		auto displayMemoryAddress = memory::getDisplayRealPage();
		displayContext.displayMemoryAddress = displayMemoryAddress;
		
		// configure AgentDisplay
		display.setDisplayType(displayContext.type);
		display.setDisplayWidth(displayContext.width);
		display.setDisplayHeight(displayContext.height);
		display.setDisplayMemoryAddress(displayMemoryAddress);
	}

	// AgentDisk
	diskFile.attach(config.diskFilePath);
	disk.addDiskFile(&diskFile);
	// AgentFloppy
	floppyFile.attach(config.floppyFilePath);
	floppy.addDiskFile(&floppyFile);
	// AgentNetwork use networkPacket
	networkPacket.attach(config.networkInterface);
	network.setNetworkPacket(&networkPacket);
	// AgentProcessor::Initialize use PID[]
	// get PID from network adapter
	networkPacket.getAddress(PID[1], PID[2], PID[3]);
	logger.info("PID               %04X-%04X-%04X", PID[1], PID[2], PID[3]);
	// set PID to AgentProcessor
	processor.setProcessorID(PID[1], PID[2], PID[3]);
	// Stream::Boot
	// bootFilePath
	// Enable Agents

	//
	// Initialize Agents
	//
	disk.Enable();
	floppy.Enable();
	network.Enable();
//	AgentParallel  parallel;
	keyboard.Enable();
	beep.Enable();
	mouse.Enable();
	processor.Enable();
	stream.Enable();
//	AgentSerial    serial;
//	AgentTTY       tty;
	display.Enable();
//	AgentReserved3 reserved3;
	logger.info("Agent FCB  %04X %04X", agent::ioRegionPage * PageSize, agent::getIORegion());

	// Initialization of Stream handler
	AgentStream* agentStream = (AgentStream*)agent::getAgent((int)GuamInputOutput::AgentDeviceIndex::stream);
	// 110 Boot
	agentStream->addStream(new StreamBoot(config.bootFilePath));
	// 101 CopyPaste
	agentStream->addStream(new StreamCopyPaste);
	//   1 PCFA
	agentStream->addStream(new StreamPCFA);
	//  21 TCP
	agentStream->addStream(new StreamTCP);
	// 108 WWC
	agentStream->addStream(new StreamWWC);

	{
		// TODO do initialization like StartPilot in PirotControl.mesa
		// TODO do initialize like ProcessorHeadGuam.mesa
		// TODO do initialize like UserTerminalHeadGuam.mesa
	}

	// load germ file into vm
	loadGerm(config.germFilePath);

	// set boot request
	{
		Boot::Request* request = (Boot::Request*)memory::peek(SD + OFFSET_SD(SDDefs::sFirstGermRequest));

		// clear boot request
		memset(request, 0, sizeof(*request));

		if (config.bootDevice == "DISK") {
			setBootRequestPV(request);
		} else if (config.bootDevice == "ETHER") {
			setBootRequestEther(request);
		} else if (config.bootDevice == "STREAM") {
			setBootRequestStream(request);
		} else {
			logger.fatal("Unknown bootDevice");
			ERROR()
		}

		setSwitches(request->switches, config.bootSwitch.c_str());
	}
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

static void boot() {
	logger.info("boot START");

	std::function<void()> f1 = std::function<void()>(interrupt_thread::run);
	std::function<void()> f2 = std::function<void()>(timer_thread::run);
	std::function<void()> f3 = std::bind(&AgentNetwork::ReceiveThread::run, &network.receiveThread);
	std::function<void()> f4 = std::bind(&AgentNetwork::TransmitThread::run, &network.transmitThread);
	std::function<void()> f5 = std::bind(&AgentDisk::IOThread::run, &disk.ioThread);
	std::function<void()> f6 = std::function<void()>(processor_thread::run);

	ThreadControl t1("interrupt", f1);
	ThreadControl t2("timer", f2);
	ThreadControl t3("receive", f3);
	ThreadControl t4("transmit", f4);
	ThreadControl t5("disk", f5);
	ThreadControl t6("processor", f6);

	auto timeStart = Util::getMilliSecondsFromEpoch();
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
	auto timeStop = Util::getMilliSecondsFromEpoch();

	elapsedTime = timeStop - timeStart;

	logger.info("boot STOP");
}

static void finalize() {
	// detach file or device
	diskFile.detach();
	floppyFile.detach();
	networkPacket.detach();

	memory::finalize();
}

void run() {
	initialize();
	boot();
	finalize();
}

int64_t getElapsedTime() {
    return elapsedTime;
}


//
// key event
//
void keyPress(int number, const std::string& string) {
	(void)string;
	const keymap::LevelVKey& levelVKey = keymap::getLevelVKey(number);
	logger.info("keyPress    %4X  %-12s  %3d %s", number, string, levelVKey.keyName, levelVKey.name);
	auto keyName = levelVKey.keyName;
	if (keyName != LevelVKeys::null) keyboard.keyPress(keyName);
}
void keyRelease(int number, const std::string& string) {
	(void)string;
	const keymap::LevelVKey& levelVKey = keymap::getLevelVKey(number);
	logger.info("keyRelease  %4X  %-12s  %3d %s", number, string, levelVKey.keyName, levelVKey.name);
	auto keyName = levelVKey.keyName;
	if (keyName != LevelVKeys::null) keyboard.keyRelease(keyName);
}

//
// mouse event
//
static std::array<LevelVKeys::KeyName, 5> buttonMap = {
	LevelVKeys::Point,
	LevelVKeys::Adjust,
	LevelVKeys::Menu,
	LevelVKeys::null,
	LevelVKeys::null,
};
void buttonPress(int number) {
	LevelVKeys::KeyName keyName = buttonMap[number];
	logger.info("buttonPress    %d  %3d", number, keyName);
	if (keyName != LevelVKeys::null) {
		keyboard.keyPress(keyName);
	} else {
		logger.error("Unexpeced mouse button  %d", number);
		ERROR();
	}
}
void buttonRelease(int number) {
	LevelVKeys::KeyName keyName = buttonMap[number];
	logger.info("buttonRelease  %d  %3d", number, keyName);
	if (keyName != LevelVKeys::null) {
		keyboard.keyRelease(keyName);
	} else {
		logger.error("Unexpeced mouse button  %d", number);
		ERROR();
	}
}
void motion(int x, int y) {
//	logger.info("motion      %4d  %4d", x, y);
	mouse.setPosition(x, y);
}

}