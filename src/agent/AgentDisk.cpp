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
// AgentDisk.cpp
//


#include "../util/Debug.h"
#include "../util/Util.h"
#include <chrono>
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"
#include "../mesa/memory.h"

#include "../mesa/processor.h"

#include "../util/DiskFile.h"

#include "AgentDisk.h"

using namespace DiskIOFaceGuam;
using namespace PilotDiskFace;

static CARD32 getBlock(DiskDCBType* dcb, DiskIOCBType* iocb) {
	CARD32 H = dcb->numberOfHeads;
	CARD32 S = dcb->sectorsPerTrack;

	return (H * iocb->diskAddress.cylinder + iocb->diskAddress.head) * S + iocb->diskAddress.sector;
}

void AgentDisk::IOThread::process(const Item& item) {
	std::chrono::steady_clock::time_point time_start;
	if (PERF_ENABLE) time_start = std::chrono::steady_clock::now();

	auto fcb = item.fcb;
	auto iocb = item.iocb;
	auto diskFile = item.diskFile;
	auto dcb = fcb->dcbs + iocb->deviceIndex;
	auto interruptSelector = fcb->interruptSelector;

	// CARD32 C = iocb->diskAddress.cylinder;
	// CARD32 H = iocb->diskAddress.head;
	// CARD32 S = iocb->diskAddress.sector;
	// CARD32 block = (C * dcb->numberOfHeads + H) * dcb->sectorsPerTrack + S;

	CARD32 block = getBlock(dcb, iocb);

	//"AGENT %s %d", name, fcb->command
	Command command = (Command)iocb->command;
	switch(command) {
	case Command::read: {
		if (DEBUG_SHOW_AGENT_DISK) logger.debug("IOThread::process %4d READ   %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

		CARD32 dataPtr = iocb->dataPtr;
		for(int i = 0; i < iocb->pageCount; i++) {
			CARD16 *buffer = memory::peek(dataPtr);
			diskFile->readPage(block++, buffer);
			dataPtr += PageSize;
		}
		//
		iocb->pageCount = 0;
		iocb->status = (CARD16)Status::goodCompletion;
		//
		PERF_COUNT(disk, read)
	}
		break;
	case Command::write: {
		if (DEBUG_SHOW_AGENT_DISK) logger.debug("IOThread::process %4d WRITE  %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

		CARD32 dataPtr = iocb->dataPtr;
		for(int i = 0; i < iocb->pageCount; i++) {
			CARD16 *buffer = memory::peek(dataPtr);
			diskFile->writePage(block++, buffer);
			dataPtr += PageSize;
		}
		//
		iocb->pageCount = 0;
		iocb->status = (CARD16)Status::goodCompletion;
		//
		PERF_COUNT(disk, write)
	}
		break;
	case Command::verify: {
		if (DEBUG_SHOW_AGENT_DISK) logger.debug("IOThread::process %4d VERIFY %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

		int ret = 0;
		CARD32 dataPtr = iocb->dataPtr;
		for(int i = 0; i < iocb->pageCount; i++) {
			CARD16 *buffer = memory::peek(dataPtr);
			ret |= diskFile->verifyPage(block++, buffer);
			dataPtr += PageSize;
		}
		//
		iocb->pageCount = 0;
		iocb->status = ret ? (CARD16)Status::dataVerifyError : (CARD16)Status::goodCompletion;
		//
		PERF_COUNT(disk, verify)
	}
		break;
	default:
		logger.fatal("command = %d", command);
		ERROR();
		break;
	}

	PERF_COUNT(disk, process)
	processor::notifyInterrupt(interruptSelector);

	if (PERF_ENABLE) {
		auto time_stop = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_stop - time_start).count();
		PERF_ADD(disk, process_time, duration)
	}
}


void AgentDisk::Initialize() {
	if (fcbAddress == 0) ERROR();
	if (diskFile == 0) ERROR();

	fcb = (DiskFCBType*)memory::peek(fcbAddress);
	fcb->nextIOCB = 0;
	fcb->interruptSelector = 0;
	fcb->stopAgent = 0;
	fcb->agentStopped = 1;
	fcb->numberOfDCBs = 1;

	if (fcb->numberOfDCBs != 1) ERROR(); // support only one disk

	// initialize dcb using diskSize
	auto diskByteSize = diskFile->getByteSize();
	auto dcb = fcb->dcbs + 0; // dcb points first entry of dcbs
	dcb->deviceType         = Device::T_anyPilotDisk;
	dcb->numberOfHeads      = DISK_NUMBER_OF_HEADS;
	dcb->sectorsPerTrack    = DISK_SECTORS_PER_TRACK;
	dcb->numberOfCylinders  = diskByteSize / (dcb->numberOfHeads * dcb->sectorsPerTrack * PAGE_SIZE_IN_BYTE);
	dcb->agentDeviceData[0] = 0;
	dcb->agentDeviceData[1] = 0;
	dcb->agentDeviceData[2] = 0;
	dcb->agentDeviceData[3] = 0;
	dcb->agentDeviceData[4] = 0;
	dcb->agentDeviceData[5] = 0;

	// sanity check
	if (diskByteSize != (CARD32)(dcb->numberOfHeads * dcb->sectorsPerTrack * dcb->numberOfCylinders * PAGE_SIZE_IN_BYTE)) ERROR();

	logger.info("AGENT %s  CHS = %5d %2d %2d  %s", name, dcb->numberOfCylinders, dcb->numberOfHeads, dcb->sectorsPerTrack, diskFile->getPath());
}

void AgentDisk::Call() {
	PERF_COUNT(agent, disk)
	if (fcb->stopAgent) {
		if (!fcb->agentStopped) {
			logger.info("AGENT %s stop", name);
		}
		fcb->agentStopped = 1;
		// Do nothing when ask to stop
		return;
	} else {
		if (fcb->agentStopped) {
			logger.info("AGENT %s start  %04X", name, fcb->interruptSelector);
			ioThread.clear();
		}
		fcb->agentStopped = 0;
	}

	if (fcb->nextIOCB == 0) {
		if (DEBUG_SHOW_AGENT_DISK) logger.debug("AGENT %s fcb->nextIOCB == 0", name);
		return; // Return if there is no IOCB
	}
	CARD32 nextIOCB = fcb->nextIOCB;
	DiskIOCBType *iocb = (DiskIOCBType *)Store(nextIOCB);
	for(;;) {
		// sanity check
		{
			auto numberOfDCBs = fcb->numberOfDCBs;
			auto deviceIndex = iocb->deviceIndex;
			if (numberOfDCBs <= deviceIndex) {
				logger.fatal("AGENT %s numberOfDCBs = %d  deviceIndex = %d", name, numberOfDCBs, deviceIndex);
				ERROR();
			}	
		}
		Command command = (Command)iocb->command;
		switch(command) {
		case Command::read:
		case Command::write:
		case Command::verify:
			break;
		default:
			logger.fatal("AGENT %s command = %d", name, command);
			ERROR();
		}

		Item item(fcb, iocb, diskFile);
		ioThread.push(item);

		if (iocb->nextIOCB == 0) break;
		// advance to next IOCB
		nextIOCB = iocb->nextIOCB;
		iocb = (DiskIOCBType *)Store(nextIOCB);
	}
}
