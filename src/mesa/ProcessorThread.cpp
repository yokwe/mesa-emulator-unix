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
// ProcessorThread.cpp
//

#include "../util/Util.h"
static log4cpp::Category& logger = Logger::getLogger("procthread");

#include "../util/Debug.h"

#include "../agent/AgentNetwork.h"
#include "../agent/AgentDisk.h"

#include "../opcode/Interpreter.h"

#include "ProcessorThread.h"
#include "InterruptThread.h"
#include "TimerThread.h"


//
// ProcessorThread
//
QWaitCondition ProcessorThread::cvRunning;
QAtomicInt     ProcessorThread::running;
int            ProcessorThread::stopThread;

int            ProcessorThread::rescheduleRequestCount = 0;
int            ProcessorThread::startRunningCount      = 0;
int            ProcessorThread::stopRunningCount       = 0;
int            ProcessorThread::abortCount             = 0;
int            ProcessorThread::rescheduleCount        = 0;

QAtomicInt     ProcessorThread::requestReschedule;
QMutex         ProcessorThread::mutexRequestReschedule;

void ProcessorThread::startRunning() {
	if (running.testAndSetOrdered(0, 1)) {
		startRunningCount++;
		return;
	}
	logger.error("startRunning testAndSetOrdered returns false  running = %d", getRunning());
}
void ProcessorThread::stopRunning() {
	if (running.testAndSetOrdered(1, 0)) {
		stopRunningCount++;
		return;
	}
	logger.error("stopRunning testAndSetOrdered returns false  running = %d", getRunning());
}
void ProcessorThread::stop() {
	logger.info("ProcessorThread::stop");
	stopThread = 1;
}
void ProcessorThread::run() {
	logger.info("ProcessorThread::run START");
	QThread::currentThread()->setPriority(PRIORITY);
	TaggedControlLink bootLink = {SD + OFFSET_SD(sBoot)};

	logger.info("bootLink  %04X %d %04X  %08X", bootLink.data, bootLink.tag, bootLink.fill, bootLink.u);
	if (!stopMessageUntilMPSet.isEmpty()) Logger::pushPriority(QtFatalMsg);

	XFER(bootLink.u, 0, XT_call, 0);
	logger.info("GFI = %04X  CB  = %08X  GF = %08X", GFI, CodeCache::CB(), GF);
	logger.info("PC  = %04X  MDS = %08X  LF = %04X", PC, Memory::MDS(), LFCache::LF());

	stopThread = 0;
	try {
		for(;;) {
			try {
				if (DEBUG_STOP_AT_NOT_RUNNING) {
					if (!getRunning()) ERROR();
				}
				Interpreter::execute();
			} catch(RequestReschedule& e) {
				rescheduleCount++;
				//logger.debug("Reschedule %-20s  %8d", e.func, rescheduleCount);
				QMutexLocker locker(&mutexRequestReschedule);
				for(;;) {
					// break if OP_STOPEMULATOR is called
					if (stopThread) goto exitLoop;

					// If not running, wait someone wake me up.
					if (!getRunning()) {
						//logger.debug("waitRunning START");
						for(;;) {
							bool ret = cvRunning.wait(&mutexRequestReschedule, WAIT_INTERVAL);
							if (ret) break;
							if (stopThread) goto exitLoop;
							//logger.debug("waitRunning WAITING");
						}
						//logger.debug("waitRunning FINISH");
					}
					// Do reschedule.
					{
						//logger.debug("reschedule START");
						// to avoid race condition of update of rescheduleFlag, guard with mutexReschedule
						int needReschedule = 0;
						const int request = getRequestReschedule();
						if (request & REQUSEST_RESCHEDULE_INTERRUPT) {
							//logger.debug("reschedule INTERRUPT");
							// process interrupt
							if (ProcessInterrupt()) needReschedule = 1;
						}
						if (request & REQUESET_RESCHEDULE_TIMER) {
							//logger.debug("reschedule TIMER");
							// process timeout
							if (TimerThread::processTimeout()) needReschedule = 1;
						}
						if (needReschedule) Reschedule(1);
						setRequestReschedule(0);
						//logger.debug("reschedule FINISH");
					}
					// It still not running, continue loop again
					if (!getRunning()) continue;
					break;
				}
			} catch(Abort& e) {
				abortCount++;
				//logger.debug("Abort %-20s  %8d", e.func, abortCount);
			}
		}
	} catch (Error& e) {
		logger.fatal("Error %-20s %4d %s", e.func, e.line, e.file);
		// Output for postmortem  examination
		logger.fatal("GFI %4X  CB %8X  PC %d", GFI, CodeCache::CB(), PC);
	}

exitLoop:
	// stop relevant thread
	AgentNetwork::ReceiveThread::stop();
	AgentNetwork::TransmitThread::stop();
	AgentDisk::IOThread::stop();
	TimerThread::stop();
	InterruptThread::stop();

	logger.info("abortCount             = %8u", abortCount);
	logger.info("rescheduleCount        = %8u", rescheduleCount);
	logger.info("rescheduleRequestCount = %8u", rescheduleRequestCount);
	logger.info("startRunningCount      = %8u", startRunningCount);
	logger.info("stopRunningCount       = %8u", stopRunningCount);
	logger.info("ProcessorThread::run STOP");
}
void ProcessorThread::requestRescheduleTimer() {
	QMutexLocker locker(&mutexRequestReschedule);
	setRequestReschedule(getRequestReschedule() | REQUESET_RESCHEDULE_TIMER);
	if (!getRunning()) cvRunning.wakeOne();
}
void ProcessorThread::requestRescheduleInterrupt() {
	QMutexLocker locker(&mutexRequestReschedule);
	setRequestReschedule(getRequestReschedule() | REQUSEST_RESCHEDULE_INTERRUPT);
	if (!getRunning()) cvRunning.wakeOne();
}


QSet<CARD16> ProcessorThread::stopAtMPSet;
QSet<CARD16> ProcessorThread::stopMessageUntilMPSet;
CARD16       ProcessorThread::mp = 0;

void ProcessorThread::stopAtMP(CARD16 newValue) {
	stopAtMPSet += newValue;
	logger.info("stopAtMP %4d", newValue);
}
CARD16 ProcessorThread::getMP() {
	return mp;
}
void ProcessorThread::setMP(CARD16 newValue) {
	mp = newValue;
	if (stopMessageUntilMPSet.contains(mp)) {
		Logger::popPriority();
		logger.info("show message at MP %4d", mp);
		// clear stopMessageUntilMPSet to prevent call twice
		stopMessageUntilMPSet.clear();
	}
	if (stopAtMPSet.contains(mp)) {
		logger.info("stop at MP %4d", mp);
		stop();
	}
}
void ProcessorThread::stopMessageUntilMP(CARD16 newValue) {
	stopMessageUntilMPSet += newValue;
	logger.info("stopMessageUntilMP %4d", newValue);

}
