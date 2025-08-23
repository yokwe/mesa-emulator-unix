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
// ProcessorThread.h
//

#pragma once

#include <atomic>
#include <set>

#include "../util/Util.h"

#include "MesaBasic.h"
#include "InterruptThread.h"

class ProcessorThread {
public:
	// Use this condition variable to wake and wait processor
	static std::condition_variable cvRunning;

	static int getRunning() {
		return running.load();
	}
	static void startRunning(); // running = 1
	static void stopRunning();  // running = 0

	static void stop();

	static void   stopAtMP(CARD16 newValue);
	static CARD16 getMP();
	static void   setMP(CARD16 newValue);
	static void   stopMessageUntilMP(CARD16 newValue);

	static void requestRescheduleTimer();
	static void requestRescheduleInterrupt();

	static int getRequestReschedule() {
		return requestReschedule.load();
	}

	static void checkRequestReschedule() {
		if (InterruptThread::isEnabled() && getRequestReschedule()) {
			rescheduleRequestCount++;
			ERROR_RequestReschedule();
		}
		// if stopThread is true, throw RequestReschedule
		if (stopThread) {
			ERROR_RequestReschedule();
		}
	}

	void run();

private:
	static std::atomic_uint running;

	static int stopThread;

	static int rescheduleRequestCount;
	static int startRunningCount;
	static int stopRunningCount;
	static int abortCount;
	static int rescheduleCount;

	//
	static const int  REQUESET_RESCHEDULE_TIMER     = 0x01;
	static const int  REQUSEST_RESCHEDULE_INTERRUPT = 0x02;
	static std::atomic_uint requestReschedule;
	static std::mutex    mutexRequestReschedule;

	static std::set<CARD16> stopAtMPSet;
	static std::set<CARD16> stopMessageUntilMPSet;
	static CARD16       mp;

	static void setRequestReschedule(int newValue) {
		requestReschedule.store(newValue);
	}
};
