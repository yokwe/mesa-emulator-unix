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
// InterruptThread.cpp
//

#include <condition_variable>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Variable.h"
#include "ProcessorThread.h"
#include "InterruptThread.h"


std::mutex        InterruptThread::mutexWP;
std::condition_variable InterruptThread::cvWP;
int            InterruptThread::stopThread;

int            InterruptThread::interruptCount    = 0;
int            InterruptThread::notifyCount       = 0;
int            InterruptThread::notifyWakeupCount = 0;

void InterruptThread::stop() {
	logger.info("InterruptThread::stop");
	stopThread = 1;
}
void InterruptThread::notifyInterrupt(CARD16 interruptSelector) {
	std::unique_lock<std::mutex> locker(mutexWP);
	notifyCount++;

	auto oldValue = WP.fetch_or(interruptSelector);

	if ((oldValue & interruptSelector) == 0) {
		// start interrupt, wake waiting thread
		cvWP.notify_one();
		notifyWakeupCount++;
	}
}
//int InterruptThread::isPending() {
//	return WP && isEnabled();
//}

void InterruptThread::run() {
	logger.info("InterruptThread::run START");

	stopThread = 0;
	std::unique_lock<std::mutex> locker(mutexWP);
	for (;;) {
		if (stopThread) break;
		interruptCount++;

		// wait until interrupt is arrived
		for(;;) {
			auto status = cvWP.wait_for(locker, Util::ONE_SECOND);
			if (status == std::cv_status::no_timeout) break;
			if (stopThread) goto exitLoop;
		}

		ProcessorThread::requestRescheduleInterrupt();
	}
exitLoop:
	logger.info("interruptCount         = %8u", interruptCount);
	logger.info("notifyCount            = %8u", notifyCount);
	logger.info("notifyWakeupCount      = %8u", notifyWakeupCount);
	logger.info("InterruptThread::run STOP");
}
