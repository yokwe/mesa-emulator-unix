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
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"
#include "../mesa/memory.h"

#include "../mesa/processor_thread.h"

#include "DiskFile.h"

#include "AgentDisk.h"

using namespace DiskIOFaceGuam;
using namespace PilotDiskFace;

static const CARD32 DEBUG_DONT_USE_THREAD = 0;

void AgentDisk::IOThread::process(const Item& item) {
	auto iocb = item.iocb;
	auto diskFile = item.diskFile;
	auto interruptSelector = item.interruptSelector;

	uint64_t time;
	(void)time;
	if (PERF_ENABLE) time = Util::getMicroSecondsSinceEpoch();
	CARD32 block = diskFile->getBlock(iocb);

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

	if (PERF_ENABLE) {
		uint64_t now = Util::getMicroSecondsSinceEpoch();
		PERF_ADD(disk, process_time, now - time)
	}

	PERF_COUNT(disk, process)
	processor_thread::notifyInterrupt(interruptSelector);
}


void AgentDisk::Initialize() {
	if (fcbAddress == 0) ERROR();

	fcb = (DiskFCBType*)memory::peek(fcbAddress);
	fcb->nextIOCB = 0;
	fcb->interruptSelector = 0;
	fcb->stopAgent = 0;
	fcb->agentStopped = 1;
	fcb->numberOfDCBs = 1;

	if (fcb->numberOfDCBs == 0) ERROR();

	dcb = fcb->dcbs;
	diskFile->setDiskDCBType(dcb);
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
		CARD16 deviceIndex = iocb->deviceIndex;
		if (fcb->numberOfDCBs <= deviceIndex) {
			logger.fatal("AGENT %s deviceIndex = %d", name, deviceIndex);
			ERROR();
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

		if (DEBUG_DONT_USE_THREAD) {
			CARD32 block = diskFile->getBlock(iocb);

			switch(command) {
			case Command::read: {
//				logger.debug("IOThread::process %4d READ   %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

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
//				logger.debug("IOThread::process %4d WRITE  %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

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
//				logger.debug("IOThread::process %4d VERIFY %08X + %3d dataPtr = %08X  nextIOCB = %08X", iocb->deviceIndex, block, iocb->pageCount, iocb->dataPtr, iocb->nextIOCB);

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
				logger.fatal("AGENT %s command = %d", name, command);
				ERROR();
			}
		} else {
			Item item(iocb, diskFile, fcb->interruptSelector);
			ioThread.push(item);
		}

		if (iocb->nextIOCB == 0) break;
		// advance to next IOCB
		nextIOCB = iocb->nextIOCB;
		iocb = (DiskIOCBType *)Store(nextIOCB);
	}

	if (DEBUG_DONT_USE_THREAD) {
		processor_thread::notifyInterrupt(fcb->interruptSelector);
	}

	// notify with interrupt
	//WP |= fcb->interruptSelector;
	//processor_thread::notifyInterrupt(fcb->interruptSelector);
}
