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
// processor.cpp
//

#include <set>
#include <mutex>
#include <condition_variable>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../agent/AgentDisk.h"
#include "../agent/AgentNetwork.h"

#include "../opcode/opcode.h"

#include "processor_thread.h"
#include "timer_thread.h"
#include "interrupt_thread.h"
#include "Variable.h"

#include "../util/Debug.h"
#include "../util/Perf.h"
#include "../util/trace.h"


namespace processor_thread {

bool                    stopThread;
std::condition_variable cvRunning;
std::mutex              mutexRequestReschedule;
VariableAtomicFlag      rescheduleInterruptFlag;
VariableAtomicFlag      rescheduleTimerFlag;
std::set<CARD16>        stopAtMPSet;


void stop() {
	logger.info("timer::stop");
    stopThread = true;
}

void stopAtMP(CARD16 mp) {
    stopAtMPSet.insert(mp);
	logger.info("stopAtMP %4d", mp);
}
void mp_observer(CARD16 mp) {
    if (stopAtMPSet.contains(mp)) {
		logger.info("stop at MP %4d", mp);
		stop();
	}
}

void requestRescheduleTimer() {
	TRACE_RECORD(requestRescheduleTimer)
    rescheduleTimerFlag.set();
	TRACE_RECORD(requestRescheduleTimer)
	if (!running) {
		TRACE_RECORD(requestRescheduleTimer)
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		TRACE_RECORD(requestRescheduleTimer)
		cvRunning.notify_one();
		TRACE_RECORD(requestRescheduleTimer)
	}
	TRACE_RECORD(requestRescheduleTimer)
}
void requestRescheduleInterrupt() {
	TRACE_RECORD(requestRescheduleInterrupt)
	rescheduleInterruptFlag.set();
	if (!running) {
		TRACE_RECORD(requestRescheduleInterrupt)
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		TRACE_RECORD(requestRescheduleInterrupt)
		cvRunning.notify_one();
		TRACE_RECORD(requestRescheduleInterrupt)
	}
	TRACE_RECORD(requestRescheduleInterrupt)
}

void checkRequestReschedule() {
	TRACE_RECORD(checkRequestReschedule)
    PERF_COUNT(processor, checkRequestReschedule)
    if (InterruptsEnabled() && (rescheduleTimerFlag || rescheduleInterruptFlag)) {
        PERF_COUNT(processor, rescheduleRequest)
		TRACE_RECORD(checkRequestReschedule)
        ERROR_RequestReschedule();
    }
    // if stopThread is true, throw RequestReschedule
    if (stopThread) {
		TRACE_RECORD(checkRequestReschedule)
        ERROR_RequestReschedule();
    }
	TRACE_RECORD(checkRequestReschedule)
}

void run() {
	TRACE_RECORD(processor)
	logger.info("processor_thread::run START");
	stopThread = 0;
	rescheduleInterruptFlag.clear();
	rescheduleTimerFlag.clear();

	TaggedControlLink bootLink = {SD + OFFSET_SD(sBoot)};
	XFER(bootLink.u, 0, XferType::call, 0);
	logger.info("bootLink  data  %04X   tag  %d", bootLink.data + 0, bootLink.tag + 0);
	logger.info("GFI = %04X  CB  = %08X  GF  = %08X", GFI, CB, GF);
	logger.info("LF  = %04X  PC  = %04X      MDS = %08X", LF, PC, MDS);

	running.timeStart();
	try {
		for(;;) {
			try {
				if (DEBUG_STOP_AT_NOT_RUNNING) {
					if (!running) ERROR();
				}
				TRACE_RECORD(processor)
				Execute();
				TRACE_RECORD(processor)
			} catch(RequestReschedule& e) {
				TRACE_RECORD(processor)
				// Only ERROR_RequestReschedule throws RequestReschedule.
				// ERROR_RequestReschedule is called from Reschedule() and processor::checkRequestReschedule().
				// In above both case, RequestReschedule will thrown while interrupt is enabled.
				// Also above both case, call is from ProcessorThread
				PERF_COUNT(processor, requestReschedule)
				//logger.debug("Reschedule %-20s  %8d", e.func, rescheduleCount);
				for(;;) {
					TRACE_RECORD(processor)
					// break if OP_STOPEMULATOR is called
					if (stopThread) goto exitLoop;

					// If not running, wait interrupts or timeouts
					if (!running) {
						//logger.debug("waitRunning START");
						TRACE_RECORD(processor)
						std::unique_lock<std::mutex> locker(mutexRequestReschedule);
						TRACE_RECORD(processor)
						for(;;) {
							TRACE_RECORD(processor)
							cvRunning.wait_for(locker, Util::ONE_SECOND);
							TRACE_RECORD(processor)
							if (stopThread) goto exitLoop;
							TRACE_RECORD(processor)
							if (rescheduleInterruptFlag) break;
							TRACE_RECORD(processor)
							if (rescheduleTimerFlag) break;
							TRACE_RECORD(processor)
							//logger.debug("waitRunning WAITING");
						}
						TRACE_RECORD(processor)
						//logger.debug("waitRunning FINISH");
					} else {
						TRACE_RECORD(processor)
						PERF_COUNT(processor, running)
					}
					// Do reschedule.
					{
						//logger.debug("reschedule START");
						bool needReschedule = false;
						TRACE_RECORD(processor)
						if (rescheduleInterruptFlag) {
							PERF_COUNT(processor, interruptFlag)
							//logger.debug("reschedule INTERRUPT");
							// process interrupt
							TRACE_RECORD(processor)
							if (Interrupt()) {
								TRACE_RECORD(processor)
								PERF_COUNT(processor, interrupt)
								needReschedule = true;
							}
							TRACE_RECORD(processor)
						}
						if (rescheduleTimerFlag) {
							PERF_COUNT(processor, timerFlag)
							//logger.debug("reschedule TIMER");
							// process timeout
							TRACE_RECORD(processor)
							if (timer_thread::processTimeout()) {
								TRACE_RECORD(processor)
								PERF_COUNT(processor, timer)
								needReschedule = true;
							}
							TRACE_RECORD(processor)
						}
						TRACE_RECORD(processor)
						if (needReschedule) {
							PERF_COUNT(processor, needReschedule)
							TRACE_RECORD(processor)
							Reschedule(1);
							TRACE_RECORD(processor)
						}
						TRACE_RECORD(processor)
						rescheduleInterruptFlag.clear();
						rescheduleTimerFlag.clear();
						//logger.debug("reschedule FINISH");
					}
					// It still not running, continue loop again
					TRACE_RECORD(processor)
					if (!running) continue;
					TRACE_RECORD(processor)
					break;
				}
			} catch(Abort& e) {
				PERF_COUNT(processor, abort)
				//logger.debug("Abort %-20s  %8d", e.func, abortCount);
			}
		}
	} catch (ErrorError& e) {
		LogSourceLocation::fatal(logger, e.location, "Error  ");
		// Output for postmortem  examination
		logger.fatal("GFI %4X  CB %8X  PC %d", GFI, CB, PC);
	}

exitLoop:
	running.timeStop();
	TRACE_RECORD(processor)

	// stop relevant thread
	AgentNetwork::ReceiveThread::stop();
	AgentNetwork::TransmitThread::stop();
	AgentDisk::IOThread::stop();
	timer_thread::stop();
	interrupt_thread::stop();

	logger.info("processor_thread::run STOP");
}

}