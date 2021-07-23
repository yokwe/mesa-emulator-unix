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
// TimerThread.cpp
//

#include "../util/Util.h"
static log4cpp::Category& logger = Logger::getLogger("timerthread");

#include "../util/Debug.h"

#include "Function.h"

#include "TimerThread.h"
#include "ProcessorThread.h"


static int timerCount = 0;

CARD16         TimerThread::PTC;
qint64         TimerThread::lastTimeoutTime;
int            TimerThread::stopThread;
QMutex         TimerThread::mutexTimer;
QWaitCondition TimerThread::cvTimer;

void TimerThread::stop() {
	logger.info("TimerThread::stop");
	stopThread = 1;
}
void TimerThread::setPTC(CARD16 newValue) {
	PTC = newValue;
	lastTimeoutTime = QDateTime::currentMSecsSinceEpoch();
}
void TimerThread::run() {
	logger.info("TimerThread::run START");
	QThread::currentThread()->setPriority(PRIORITY);

	lastTimeoutTime = QDateTime::currentMSecsSinceEpoch();
	stopThread = 0;
	QMutexLocker locker(&mutexTimer);
	for (;;) {
		if (stopThread) break;
		timerCount++;
		qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
		// I will wait until TIMER_INTERVAL is elapsed since preveiousTime.
		qint64 waitTime = lastTimeoutTime + TIMER_INTERVAL - currentTime;
		if (waitTime < 0) {
			// already elapsed
			if (-waitTime < TIMER_INTERVAL) {
				// Time difference is not significant. Hope it will recover.
			} else {
				// Time difference is significant. Assign value to lastTimeoutTime.
				lastTimeoutTime = currentTime;
				logger.warn("Timer lost.  lost = %d ms",(quint32) (-waitTime));
			}
		} else {
			// need to wait until TIMER_INTERVAL is elapsed since preveiousTime.
			Util::msleep((quint32) waitTime);
		}

		{
			ProcessorThread::requestRescheduleTimer();
			// wait procesTimeout is invoked
			for(;;) {
				bool ret = cvTimer.wait(&mutexTimer, WAIT_INTERVAL);
				if (ret) break;
				if (stopThread) goto exitLoop;
			}
		}
	}
exitLoop:
	logger.info("TimerThread::run STOP");
}

int TimerThread::processTimeout() {
	QMutexLocker locker(&mutexTimer);
	//logger.debug("processTimeout START");
	// Don't update lastTimeoutTimer, until actual process is performed
	lastTimeoutTime += TIMER_INTERVAL;

	PTC = PTC + 1;
	if (PTC == 0) PTC = PTC + 1;

	int ret = TimeoutScan();
	cvTimer.wakeOne();
	//logger.debug("processTimeout FINISH");
	return ret;
}
