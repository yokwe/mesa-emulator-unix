/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
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

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

#include "../util/Debug.h"

#include "InterruptThread.h"
#include "ProcessorThread.h"


CARD16         InterruptThread::WP;
CARD16         InterruptThread::WDC;
QMutex         InterruptThread::mutexWP;
QWaitCondition InterruptThread::cvWP;
int            InterruptThread::stopThread;

int            InterruptThread::interruptCount    = 0;
int            InterruptThread::notifyCount       = 0;
int            InterruptThread::notifyWakeupCount = 0;

void InterruptThread::stop() {
	logger.info("InterruptThread::stop");
	stopThread = 1;
}
CARD16 InterruptThread::getWP() {
	return WP;
}
void InterruptThread::setWP(CARD16 newValue) {
	QMutexLocker locker(&mutexWP);
	CARD16 oldValue = WP;
	WP = newValue;
	if (oldValue && !newValue) {
		// become no interrupt
	} else if (!oldValue && newValue) {
		// start interrupt, wake waiting thread
		cvWP.wakeOne();
	}
}
void InterruptThread::notifyInterrupt(CARD16 interruptSelector) {
	QMutexLocker locker(&mutexWP);
	notifyCount++;
	CARD16 newValue = (WP | interruptSelector);
	//
	CARD16 oldValue = WP;
	WP = newValue;
	if (oldValue && !newValue) {
		// become no interrupt
	} else if (!oldValue && newValue) {
		// start interrupt, wake waiting thread
		cvWP.wakeOne();
		notifyWakeupCount++;
	}
}
//int InterruptThread::isPending() {
//	return WP && isEnabled();
//}

void InterruptThread::run() {
	logger.info("InterruptThread::run START");
	QThread::currentThread()->setPriority(PRIORITY);

	stopThread = 0;
	QMutexLocker locker(&mutexWP);
	for (;;) {
		if (stopThread) break;
		interruptCount++;

		// wait until interrupt is arrived
		for(;;) {
			bool ret = cvWP.wait(&mutexWP, WAIT_INTERVAL);
			if (ret) break;
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
