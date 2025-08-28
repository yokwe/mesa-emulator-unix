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

#include "Function.h"
#include "TimerThread.h"
#include "ProcessorThread.h"


CARD16         TimerThread::PTC;
CARD32         TimerThread::lastTimeoutTime;
int            TimerThread::stopThread;
std::mutex        TimerThread::mutexTimer;
std::condition_variable TimerThread::cvTimer;
int            TimerThread::timerCount = 0;

void TimerThread::stop() {
	logger.info("TimerThread::stop");
	stopThread = 1;
}
void TimerThread::setPTC(CARD16 newValue) {
	PTC = newValue;
	lastTimeoutTime = IT;
}

void TimerThread::run() {
	logger.info("TimerThread::run START");

	lastTimeoutTime = IT;
	stopThread = 0;
	std::unique_lock<std::mutex> locker(mutexTimer);
	for (;;) {
		if (stopThread) break;
		timerCount++;
		CARD32 currentTime = IT;
		// I will wait until TIMER_INTERVAL is elapsed since preveiousTime.
		int64_t waitTime = lastTimeoutTime + TIMER_INTERVAL - currentTime;
		if (waitTime < 0) {
			// already elapsed
			if (-waitTime < TIMER_INTERVAL) {
				// Time difference is not significant. Hope it will recover.
			} else {
				// Time difference is significant. Assign value to lastTimeoutTime.
				lastTimeoutTime = currentTime;
				logger.warn("Timer lost.  lost = %d ms",(uint32_t) (-waitTime));
			}
		} else {
			// need to wait until TIMER_INTERVAL is elapsed since preveiousTime.
			std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
		}

		{
			ProcessorThread::requestRescheduleTimer();
			// wait procesTimeout is invoked
			for(;;) {
				auto status = cvTimer.wait_for(locker, Util::ONE_SECOND);
				if (status == std::cv_status::no_timeout) break;
				if (stopThread) goto exitLoop;
			}
		}
	}
exitLoop:
	logger.info("timerCount             = %8u", timerCount);
	logger.info("TimerThread::run STOP");
}

int TimerThread::processTimeout() {
	std::unique_lock<std::mutex> locker(mutexTimer);
	//logger.debug("processTimeout START");
	// Don't update lastTimeoutTimer, until actual process is performed
	lastTimeoutTime += TIMER_INTERVAL;

	// FIXME PTC update only interrupt is enabled.  See CheckForTimeouts

	PTC = PTC + 1;
	if (PTC == 0) PTC = PTC + 1;

	int ret = TimeoutScan();
	cvTimer.notify_one();
	//logger.debug("processTimeout FINISH");
	return ret;
}
