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
// DiskFile.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "DiskFile.h"

void DiskFile::readPage(CARD32 block, CARD16 *buffer, CARD32 sizeInWord) {
	if (maxBlock <= block) {
		logger.fatal("block = %d  maxBlock = %d", block, maxBlock);
		ERROR();
	}
	memcpy(buffer, page + block, sizeInWord * Environment::bytesPerWord);
}
void DiskFile::writePage(CARD32 block, CARD16 *buffer, CARD32 sizeInWord) {
	if (maxBlock <= block) {
		logger.fatal("block = %d  maxBlock = %d", block, maxBlock);
		ERROR();
	}
	memcpy(page + block, buffer, sizeInWord * Environment::bytesPerWord);
}
void DiskFile::zeroPage(CARD32 block) {
	if (maxBlock <= block) {
		logger.fatal("block = %d  maxBlock = %d", block, maxBlock);
		ERROR();
	}
	bzero(page + block, SIZE(Page));
}
int DiskFile::verifyPage(CARD32 block, CARD16 *buffer) {
	if (maxBlock <= block) {
		logger.fatal("block = %d  maxBlock = %d", block, maxBlock);
		ERROR();
	}
	return memcmp(page + block, buffer, sizeof(Page));
}

void DiskFile::attach(const std::string& path_) {
	path = path_;
	logger.info("DiskFile::attach %s", path);

	page = (Page*)Util::mapFile(path, size);
	maxBlock = getBlockSize();
}

void DiskFile::detach() {
	logger.info("DiskFile::detach %s", path);

	Util::unmapFile(page);
	page              = 0;
	size              = 0;
	maxBlock          = 0;
	numberOfCylinders = 0;
	numberOfHeads     = 0;
	sectorsPerTrack   = 0;
}

void DiskFile::setDiskDCBType(DiskIOFaceGuam::DiskDCBType *dcb) {
	dcb->deviceType         = Device::T_anyPilotDisk;
	dcb->numberOfHeads      = DISK_NUMBER_OF_HEADS;
	dcb->sectorsPerTrack    = DISK_SECTORS_PER_TRACK;
	dcb->numberOfCylinders  = size / (dcb->numberOfHeads * dcb->sectorsPerTrack * sizeof(Page));
	dcb->agentDeviceData[0] = 0;
	dcb->agentDeviceData[1] = 0;
	dcb->agentDeviceData[2] = 0;
	dcb->agentDeviceData[3] = 0;
	dcb->agentDeviceData[4] = 0;
	dcb->agentDeviceData[5] = 0;

	if (size != (CARD32)(dcb->numberOfHeads * dcb->sectorsPerTrack * dcb->numberOfCylinders * sizeof(Page))) ERROR();

	numberOfCylinders = dcb->numberOfCylinders;
	numberOfHeads     = dcb->numberOfHeads;
	sectorsPerTrack   = dcb->sectorsPerTrack;
}

void DiskFile::setFloppyDCBType(FloppyIOFaceGuam::FloppyDCBType *dcb) {
	dcb->deviceType         = Device::T_microFloppy; // Don't use Device::T_anyFloppy
	dcb->numberOfHeads      = FLOPPY_NUMBER_OF_HEADS;
	dcb->sectorsPerTrack    = FLOPPY_SECTORS_PER_TRACK;
	dcb->numberOfCylinders  = size / (dcb->numberOfHeads * dcb->sectorsPerTrack * sizeof(Page));
	dcb->ready              = 1;
	dcb->diskChanged        = 1;
	dcb->twoSided           = 1;
	dcb->suggestedTries     = 1;

	if (size != (CARD32)(dcb->numberOfHeads * dcb->sectorsPerTrack * dcb->numberOfCylinders * sizeof(Page))) ERROR();

	numberOfCylinders = dcb->numberOfCylinders;
	numberOfHeads     = dcb->numberOfHeads;
	sectorsPerTrack   = dcb->sectorsPerTrack;
}
