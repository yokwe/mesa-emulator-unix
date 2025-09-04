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
// interrupt.cpp
//

#include <condition_variable>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/Perf.h"

#include "Variable.h"
#include "ProcessorThread.h"
#include "interrupt.h"


namespace interrupt {

std::mutex              mutexWP;
std::condition_variable cvWP;
bool                    stopThread;

void stop() {
	logger.info("interrupt::stop");
    stopThread = true;
}

void run() {
	logger.info("interrup::run START");

	stopThread = 0;
	std::unique_lock<std::mutex> locker(mutexWP);
	for (;;) {
		if (stopThread) break;
		PERF_COUNT(interrupt, interrupt)

		// wait until interrupt is arrived
		for(;;) {
			cvWP.wait_for(locker, Util::ONE_SECOND);
			if (stopThread) goto exitLoop;
			if (WP.pending()) break;
		}
		PERF_COUNT(interrupt, request)
		ProcessorThread::requestRescheduleInterrupt();
	}
exitLoop:
	logger.info("interrupt::run STOP");
}

void notifyInterrupt(CARD16 interruptSelector) {
	PERF_COUNT(interrupt, notify)

	auto oldValue = WP.fetch_or(interruptSelector);

	if (interruptSelector && (oldValue & interruptSelector) == 0) {
		std::unique_lock<std::mutex> locker(mutexWP);
		// start interrupt, wake waiting thread
		cvWP.notify_one();
		PERF_COUNT(interrupt, wakeup)
	}
}

}
