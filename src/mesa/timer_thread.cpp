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

#include <atomic>
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
std::atomic_bool        cvTimerFlag;


void stop() {
	logger.info("timer_thread::stop");
    stopThread = true;
}

void run() {
	TRACE_REC_(timer, run)
	logger.info("timer_thread::run START");
	stopThread = false;
	cvTimerFlag = false;
	
	auto tick = std::chrono::milliseconds(cTick);
	auto time = std::chrono::system_clock::now();
	for(;;) {
		PERF_COUNT(timer, timer)
		TRACE_REC_(timer, timer)
		auto nextTime = time + tick;
		TRACE_REC_(timer, sleep_until_start)
		std::this_thread::sleep_until(nextTime);
		TRACE_REC_(timer, sleep_until_stop)
		time = nextTime;
		if (stopThread) break;

		// processor::requestRescheduleTimer() will call TimerThread::processTimeout() eventually
		// Then TimerThread::processTimeout() will notify cvTimer
		TRACE_REC_(timer, requestRescheduleTimer_calling)
		processor_thread::requestRescheduleTimer();
		TRACE_REC_(timer, requestRescheduleTimer_called)
		// wait procesTimeout is invoked
		for(;;) {
			TRACE_REC_(timer, mutexTimer_locking)
			std::unique_lock<std::mutex> locker(mutexTimer);
			TRACE_REC_(timer, mutexTimer_locked)
			TRACE_REC_(timer, cvTimer_wait_for_calling)
			cvTimer.wait_for(locker, Util::ONE_SECOND);
			TRACE_REC_(timer, cvTimer_wait_for_called)
			if (stopThread) goto exitLoop;
			if (cvTimerFlag) break;
		}
		TRACE_REC_(timer, exit_inner_for_loop)
		cvTimerFlag = false;
	}
exitLoop:
	logger.info("timer_thread::run STOP");
}

// To process timeout in other thread, create processTimeout
bool processTimeout() {
	// this method is called from processor thread
	//logger.debug("processTimeout START");
	PERF_COUNT(timer, processTimeout_ENTER)
	TRACE_REC_(timer, processTimeout_ENTER)
	{
		// start next timer
		TRACE_REC_(timer, mutexTimer_locking)
		std::unique_lock<std::mutex> locker(mutexTimer);
		cvTimerFlag = true;
		TRACE_REC_(timer, mutexTimer_locked)
		TRACE_REC_(timer, cvTimer_notifying)
		cvTimer.notify_one();
		TRACE_REC_(timer, cvTimer_notified)
	}

	bool requeue;
	TRACE_REC_(timer, Interrupt)
	if (InterruptsEnabled()) {
		PERF_COUNT(timer, updatePTC)
		TRACE_REC_(timer, updatePTC)
		PTC = PTC + 1;
		if (PTC == 0) PTC = PTC + 1;

		TRACE_REC_(timer, TimeoutScan_calling)
		requeue = TimeoutScan();
		TRACE_REC_(timer, TimeoutScan_called)
	} else {
		requeue = false;
	}
	PERF_COUNT(timer, processTimeout_EXIT)
	TRACE_REC_(timer, processTimeout_EXIT)
	return requeue;
}

}