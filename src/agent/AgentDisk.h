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
// AgentDisk.h
//

#pragma once

#include "../mesa/Pilot.h"

#include "Agent.h"
#include "DiskFile.h"

#include "../util/ThreadQueue.h"

class AgentDisk : public Agent {
public:
	class Item {
	public:
		DiskIOFaceGuam::DiskIOCBType* iocb;
		DiskFile*                     diskFile;
		CARD16                        interruptSelector;

		Item(DiskIOFaceGuam::DiskIOCBType* iocb_, DiskFile* diskFile_, CARD16 interruptSelector_) : iocb(iocb_), diskFile(diskFile_), interruptSelector(interruptSelector_) {}
		Item(const Item& that) : iocb(that.iocb), diskFile(that.diskFile), interruptSelector(that.interruptSelector) {}
	};

	class IOThread : public thread_queue::ThreadQueueProcessor<Item> { 
	public:
		IOThread() : thread_queue::ThreadQueueProcessor<Item>("disk") {}
		void process(const Item& data);
	};

	IOThread ioThread;

	static const inline auto index_ = GuamInputOutput::AgentDeviceIndex::disk;
	static const inline auto name_ = "Disk";
	static const inline auto fcbSize_ = SIZE(DiskIOFaceGuam::DiskFCBType) + SIZE(DiskIOFaceGuam::DiskDCBType);
	AgentDisk() : Agent(index_, name_, fcbSize_) {
		fcb      = 0;
		dcb      = 0;
		diskFile = 0;
	}

	void Initialize();
	void Call();

	void addDiskFile(DiskFile* diskFile_) {
		diskFile = diskFile_;
	}

private:
	DiskIOFaceGuam::DiskFCBType* fcb      = 0;
	DiskIOFaceGuam::DiskDCBType* dcb      = 0;
	DiskFile*                    diskFile = 0;;
};
