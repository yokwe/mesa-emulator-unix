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
// TimerThread.cpp
//

#include <chrono>
#include <thread>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/Perf.h"

#include "Function.h"
#include "ProcessorThread.h"
#include "Variable.h"
#include "TimerThread.h"


std::mutex              TimerThread::mutexTimer;
std::condition_variable TimerThread::cvTimer;

int      TimerThread::stopThread;
uint64_t TimerThread::lastTimeoutTime;

void TimerThread::stop() {
	logger.info("TimerThread::stop");
	stopThread = 1;
}

void TimerThread::run() {
	logger.info("TimerThread::run START");

	auto tick = std::chrono::milliseconds(cTick);
	auto time = std::chrono::system_clock::now();
	std::unique_lock<std::mutex> locker(mutexTimer);
	for(;;) {
		PERF_COUNT(timer, timer)
		auto nextTime = time + tick;
		std::this_thread::sleep_until(nextTime);
		time = nextTime;
		if (stopThread) break;

		{
			// ProcessorThread::requestRescheduleTimer() will call TimerThread::processTimeout() eventually
			// Then TimerThread::processTimeout() will notify cvTimer
			ProcessorThread::requestRescheduleTimer();
			// wait procesTimeout is invoked
			for(;;) {
				auto status = cvTimer.wait_for(locker, Util::ONE_SECOND);
				if (stopThread) goto exitLoop;
				if (status == std::cv_status::no_timeout) break;
			}
		}
	}
exitLoop:
	logger.info("TimerThread::run STOP");
}

bool TimerThread::processTimeout() {
	// this method is called from processor thread
	//logger.debug("processTimeout START");
	PERF_COUNT(timer, timeout)
	{
		// start next timer
		std::unique_lock<std::mutex> locker(mutexTimer);
		cvTimer.notify_one();
	}

	bool requeue;
	if (InterruptsEnabled()) {
		PERF_COUNT(timer, updatePTC)
		PTC = PTC + 1;
		if (PTC == 0) PTC = PTC + 1;

		requeue = TimeoutScan();
		//logger.debug("processTimeout FINISH");
	} else {
		requeue = false;
	}
	return requeue;
}
