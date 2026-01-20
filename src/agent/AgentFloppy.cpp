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
// AgentFloppy.cpp
//

#include "../util/Debug.h"
#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"
#include "../mesa/memory.h"
#include "../mesa/processor.h"

#include "AgentFloppy.h"



//SectorToDiskAddress: PROCEDURE [
//   sector: Sector, cylinders: CARDINAL, tracksPerCylinder: CARDINAL,
//   sectorsPerTrack: CARDINAL]
//   RETURNS [diskAddress: FloppyChannel.DiskAddress] = INLINE
//   BEGIN
//   temp: CARDINAL;
//   -- Remember that disk sectors are numbered [1..sectors per track] and that
//   -- Sector 0 is unused while Sector 1 is address [sector: 1, head: 0, cyl: 0]
//   [quotient: temp, remainder: diskAddress.sector] =
//     Inline.DIVMOD[sector-1, sectorsPerTrack];
//   diskAddress.sector = diskAddress.sector+1; -- sectors begin at 1
//   [quotient: diskAddress.cylinder, remainder: diskAddress.head] =
//      Inline.DIVMOD[temp, tracksPerCylinder];
//   END;
//
static FloppyDiskFace::DiskAddress sectorToDiskAddress(FloppyIOFaceGuam::FloppyDCBType* dcb, CARD32 sector) {
	const CARD32 tracksPerCylinder = dcb->numberOfHeads;
	const CARD32 sectorsPerTrack   = dcb->sectorsPerTrack;

	FloppyDiskFace::DiskAddress diskAddress;
	CARD32 temp          = ((sector - 1) / sectorsPerTrack);
	diskAddress.sector   = ((sector - 1) % sectorsPerTrack) + 1;
	diskAddress.cylinder = temp / tracksPerCylinder;
	diskAddress.head     = temp % tracksPerCylinder;
	return diskAddress;
}

//DiskAddressToSector: PROCEDURE [
//   diskAddress: FloppyChannel.DiskAddress, cylinders: CARDINAL,
//   tracksPerCylinder: CARDINAL, sectorsPerTrack: CARDINAL]
//   RETURNS [sector: Sector] = INLINE
//   {RETURN[
//      diskAddress.sector + sectorsPerTrack*
//	(diskAddress.head + tracksPerCylinder * diskAddress.cylinder)]};
static CARD32 diskAddressToSector(FloppyIOFaceGuam::FloppyDCBType* dcb, FloppyDiskFace::DiskAddress diskAddress) {
	const CARD32 tracksPerCylinder = dcb->numberOfHeads;
	const CARD32 sectorsPerTrack   = dcb->sectorsPerTrack;

	return diskAddress.sector + sectorsPerTrack * (diskAddress.head + tracksPerCylinder * diskAddress.cylinder);
}


void AgentFloppy::Initialize() {
	if (fcbAddress == 0) ERROR();

	fcb = (FloppyIOFaceGuam::FloppyFCBType *)memory::peek(fcbAddress);
	fcb->nextIOCB = 0;
	fcb->interruptSelector = 0;
	fcb->stopAgent = 0;
	fcb->agentStopped = 1;
	fcb->numberOfDCBs = 1;

	if (fcb->numberOfDCBs != 1) ERROR(); // support only one disk

	// initialize dcb using diskSize
	auto diskByteSize = diskFile->getByteSize();
	auto dcb = fcb->dcbs + 0; // dcb points first entry of dcbs
	dcb->deviceType         = Device::T_microFloppy; // Don't use Device::T_anyFloppy
	dcb->numberOfHeads      = FLOPPY_NUMBER_OF_HEADS;
	dcb->sectorsPerTrack    = FLOPPY_SECTORS_PER_TRACK;
	dcb->numberOfCylinders  = diskByteSize / (dcb->numberOfHeads * dcb->sectorsPerTrack * PAGE_SIZE_IN_BYTE);
	dcb->ready              = 1;
	dcb->diskChanged        = 1;
	dcb->twoSided           = 1;
	dcb->suggestedTries     = 1;

	// sanity check
	if (diskByteSize != (CARD32)(dcb->numberOfHeads * dcb->sectorsPerTrack * dcb->numberOfCylinders * PAGE_SIZE_IN_BYTE)) ERROR();

	logger.debug("AGENT %s  CHS = %5d %2d %2d  %s", name, dcb->numberOfCylinders, dcb->numberOfHeads, dcb->sectorsPerTrack, diskFile->getPath());
}

void AgentFloppy::Call() {
	PERF_COUNT(agent, floppy)
	if (fcb->stopAgent) {
		if (!fcb->agentStopped) {
			logger.info("AGENT %s stop", name);
		}
		fcb->agentStopped = 1;
		// Do nothing when ask to stop
		return;
	} else {
		if (fcb->agentStopped) {
			logger.info("AGENT %s start", name);
		}
		fcb->agentStopped = 0;
	}

	if (fcb->nextIOCB == 0) {
		if (DEBUG_SHOW_AGENT_FLOPPY) logger.debug("AGENT %s fcb->nextIOCB == 0", name);
		return; // Return if there is no IOCB
	}
	FloppyIOCBType *iocb = (FloppyIOCBType *)Store(fcb->nextIOCB);
	for(;;) {
		const CARD16 deviceIndex = iocb->operation.device;
		if (fcb->numberOfDCBs <= deviceIndex) {
			logger.fatal("numberOfDCBs = %d  deviceIndex = %d", fcb->numberOfDCBs, deviceIndex);
			ERROR();
		}
		FloppyIOFaceGuam::FloppyDCBType* dcb = fcb->dcbs + deviceIndex;
		const CARD32 sectorNo = diskAddressToSector(dcb, iocb->operation.address);
		
		//"AGENT %s %d", name, fcb->command
		FloppyDiskFace::Function command = (FloppyDiskFace::Function)iocb->operation.function;
		switch(command) {
		case FloppyDiskFace::Function::nop:
			if (DEBUG_SHOW_AGENT_FLOPPY)
				logger.debug("AGENT %s %1d NOP    %8X (%02d-%d-%02d) + %4d %3d %d  dataPtr = %08X  nextIOCB = %08X",
					name, deviceIndex, sectorNo,
					iocb->operation.address.cylinder, iocb->operation.address.head + 0, iocb->operation.address.sector + 0,
					iocb->operation.count, iocb->sectorLength, iocb->operation.incrementDataPointer + 0,
					iocb->operation.dataPtr, fcb->nextIOCB);
			iocb->status = (CARD16)FloppyDiskFace::Status::goodCompletion;
			break;
		case FloppyDiskFace::Function::readSector: {
			if (DEBUG_SHOW_AGENT_FLOPPY)
				logger.debug("AGENT %s %1d READ   %8X (%02d-%d-%02d) + %4d %3d %d  dataPtr = %08X  nextIOCB = %08X",
					name, deviceIndex, sectorNo,
					iocb->operation.address.cylinder, iocb->operation.address.head + 0, iocb->operation.address.sector + 0,
					iocb->operation.count, iocb->sectorLength, iocb->operation.incrementDataPointer + 0,
					iocb->operation.dataPtr, fcb->nextIOCB);
			if (PageSize < iocb->sectorLength) {
				logger.fatal("PageSize = %d  iocb->sectorLength = %d", PageSize, iocb->sectorLength);
				ERROR();
			}

			for(int i = 0; i < iocb->operation.count; i++) {
				CARD32 sector = diskAddressToSector(dcb, iocb->operation.address);
				CARD16 *buffer = Store(iocb->operation.dataPtr);
				diskFile->readPage(sector - 1, buffer);
				if (iocb->operation.incrementDataPointer) {
					iocb->operation.dataPtr += iocb->sectorLength;
				}
				sector++;
				iocb->operation.address = sectorToDiskAddress(dcb, sector);
			}
			iocb->operation.count = 0;
			iocb->status = (CARD16)FloppyDiskFace::Status::goodCompletion;
		}
			break;
		case FloppyDiskFace::Function::writeSector: {
			if (DEBUG_SHOW_AGENT_FLOPPY)
				logger.debug("AGENT %s %1d WRITE  %8X (%02d-%d-%02d) + %4d %3d %d  dataPtr = %08X  nextIOCB = %08X",
					name, deviceIndex, sectorNo,
					iocb->operation.address.cylinder, iocb->operation.address.head + 0, iocb->operation.address.sector + 0,
					iocb->operation.count, iocb->sectorLength, iocb->operation.incrementDataPointer + 0,
					iocb->operation.dataPtr, fcb->nextIOCB);
			if (PageSize < iocb->sectorLength) {
				logger.fatal("PageSize = %d  iocb->sectorLength = %d", PageSize, iocb->sectorLength);
				ERROR();
			}

			for(int i = 0; i < iocb->operation.count; i++) {
				CARD32 sector = diskAddressToSector(dcb, iocb->operation.address);
				CARD16 *buffer = Fetch(iocb->operation.dataPtr);
				diskFile->writePage(sector - 1, buffer);
				if (iocb->operation.incrementDataPointer) {
					iocb->operation.dataPtr += iocb->sectorLength;
					sector++;
					iocb->operation.address = sectorToDiskAddress(dcb, sector);
				}
			}
			iocb->operation.count = 0;
			iocb->status = (CARD16)FloppyDiskFace::Status::goodCompletion;
		}
			break;
		case FloppyDiskFace::Function::formatTrack: {
			if (DEBUG_SHOW_AGENT_FLOPPY)
				logger.debug("AGENT %s %1d FORMAT %8X (%02d-%d-%02d) + %4d %3d %d  dataPtr = %08X  nextIOCB = %08X",
					name, deviceIndex, sectorNo,
					iocb->operation.address.cylinder, iocb->operation.address.head + 0, iocb->operation.address.sector + 0,
					iocb->operation.count, iocb->sectorLength, iocb->operation.incrementDataPointer + 0,
					iocb->operation.dataPtr, fcb->nextIOCB);
			CARD32 sector = diskAddressToSector(dcb, iocb->operation.address);
			CARD32 count = iocb->operation.count * iocb->sectorsPerTrack;
			for(CARD32 i = 0; i < count; i++) {
				diskFile->zeroPage(sector - 1);
				sector++;
			}
			iocb->operation.count = 0;
			iocb->status = (CARD16)FloppyDiskFace::Status::goodCompletion;
		}
			break;
		default:
			logger.debug("AGENT %s %1d %6d %8X (%02d-%d-%02d) + %4d %3d %d  dataPtr = %08X  nextIOCB = %08X",
				name, deviceIndex, command, sectorNo,
				iocb->operation.address.cylinder, iocb->operation.address.head + 0, iocb->operation.address.sector + 0,
				iocb->operation.count, iocb->sectorLength, iocb->operation.incrementDataPointer + 0,
				iocb->operation.dataPtr, fcb->nextIOCB);
			ERROR();
			break;
		}

		if (iocb->nextIOCB == 0) break;
		// advance to next IOCB
		iocb = (FloppyIOFaceGuam::FloppyIOCBType *)Store(iocb->nextIOCB);
	}

	// notify with interrupt
	processor::notifyInterrupt(fcb->interruptSelector);
}
