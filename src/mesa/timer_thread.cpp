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
// timer_thread.cpp
//

#include <chrono>
#include <thread>

#include "../util/Perf.h"
#include "../util/trace.h"

#include "Constant.h"
#include "Variable.h"
#include "processor_thread.h"
#include "timer_thread.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

namespace timer_thread {

bool                    stopThread;
std::mutex              mutexTimer;
std::condition_variable cvTimer;

void stop() {
	logger.info("timer_thread::stop");
    stopThread = true;
}

void run() {
	TRACE_RECORD(timer, run)
	logger.info("timer_thread::run START");
	stopThread = false;
	
	auto tick = std::chrono::milliseconds(cTick);
	auto time = std::chrono::system_clock::now();
	std::unique_lock<std::mutex> locker(mutexTimer);
	for(;;) {
		TRACE_RECORD(timer, run)
		PERF_COUNT(timer, timer)
		auto nextTime = time + tick;
		std::this_thread::sleep_until(nextTime);
		time = nextTime;
		if (stopThread) break;

		{
			// processor::requestRescheduleTimer() will call TimerThread::processTimeout() eventually
			// Then TimerThread::processTimeout() will notify cvTimer
			TRACE_RECORD(timer, run)
			processor_thread::requestRescheduleTimer();
			// wait procesTimeout is invoked
			for(;;) {
				TRACE_RECORD(timer, run)
				auto status = cvTimer.wait_for(locker, Util::ONE_SECOND);
				TRACE_RECORD(timer, run)
				if (stopThread) goto exitLoop;
				if (status == std::cv_status::no_timeout) break;
			}
		}
	}
exitLoop:
	TRACE_RECORD(timer, run)
	logger.info("timer_thread::run STOP");
}

// To process timeout in other thread, create processTimeout
bool processTimeout() {
	// this method is called from processor thread
	//logger.debug("processTimeout START");
	PERF_COUNT(timer, timeout)
	{
		// start next timer
		TRACE_RECORD(timer, processTimeout)
		std::unique_lock<std::mutex> locker(mutexTimer);
		TRACE_RECORD(timer, processTimeout)
		cvTimer.notify_one();
	}

	bool requeue;
	if (InterruptsEnabled()) {
		TRACE_RECORD(timer, processTimeout)
		PERF_COUNT(timer, updatePTC)
		PTC = PTC + 1;
		if (PTC == 0) PTC = PTC + 1;

		requeue = TimeoutScan();
	} else {
		requeue = false;
	}
	return requeue;
}

}