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
#include "../util/trace.h"

#include "Variable.h"
#include "processor_thread.h"
#include "interrupt_thread.h"

namespace interrupt_thread {

bool                    stopThread;
std::mutex              mutexWP;
std::condition_variable cvWP;

void stop() {
	logger.info("interrupt_thread::stop");
    stopThread = true;
}

void run() {
	TRACE_RECORD(interrupt)
	logger.info("interrup_thread::run START");
	stopThread = false;
	
	std::unique_lock<std::mutex> locker(mutexWP);
	for (;;) {
		if (stopThread) break;
		PERF_COUNT(interrupt, interrupt)

		// wait until interrupt is arrived
		for(;;) {
			TRACE_RECORD(interrupt)
			cvWP.wait_for(locker, Util::ONE_SECOND);
			TRACE_RECORD(interrupt)
			if (stopThread) goto exitLoop;
			TRACE_RECORD(interrupt)
			if (WP.pending()) break;
			TRACE_RECORD(interrupt)
		}
		PERF_COUNT(interrupt, request)
		TRACE_RECORD(interrupt)
		processor_thread::requestRescheduleInterrupt();
		TRACE_RECORD(interrupt)
	}
exitLoop:
	TRACE_RECORD(interrupt)
	logger.info("interrupt_thread::run STOP");
}

void notifyInterrupt(CARD16 interruptSelector) {
	PERF_COUNT(interrupt, notify)
	TRACE_RECORD(notifyInterrupt)

	auto oldValue = WP.fetch_or(interruptSelector);

	if (interruptSelector && (oldValue & interruptSelector) == 0) {
		TRACE_RECORD(notifyInterrupt)
		std::unique_lock<std::mutex> locker(mutexWP);
		TRACE_RECORD(notifyInterrupt)
		// start interrupt, wake waiting thread
		cvWP.notify_one();
		TRACE_RECORD(notifyInterrupt)
		PERF_COUNT(interrupt, wakeup)
	}
	TRACE_RECORD(notifyInterrupt)
}

}
