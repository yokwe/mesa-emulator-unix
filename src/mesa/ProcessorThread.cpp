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
// ProcessorThread.cpp
//

#include "../util/Util.h"
#include "Variable.h"
static const Logger logger(__FILE__);

#include "../agent/AgentNetwork.h"
#include "../agent/AgentDisk.h"
#include "../opcode/Interpreter.h"
#include "../util/Debug.h"
#include "../util/Perf.h"

#include "ProcessorThread.h"
#include "InterruptThread.h"
#include "TimerThread.h"


//
// ProcessorThread
//
std::condition_variable ProcessorThread::cvRunning;

int ProcessorThread::stopThread                  = 0;

std::mutex         ProcessorThread::mutexRequestReschedule;
VariableAtomicFlag ProcessorThread::rescheduleInterruptFlag;
VariableAtomicFlag ProcessorThread::rescheduleTimerFlag;


void ProcessorThread::stop() {
	logger.info("ProcessorThread::stop");
	stopThread = 1;
}

void ProcessorThread::run() {
	logger.info("ProcessorThread::run START");

	TaggedControlLink bootLink = {SD + OFFSET_SD(sBoot)};

	logger.info("bootLink  %04X %d %04X  %08X", bootLink.data + 0, bootLink.tag + 0, bootLink.fill, bootLink.u);

	XFER(bootLink.u, 0, XferType::call, 0);
	logger.info("GFI = %04X  CB  = %08X  GF  = %08X", GFI, CB, GF);
	logger.info("LF  = %04X  PC  = %04X      MDS = %08X", LF, PC, Memory::MDS());

	rescheduleInterruptFlag.clear();
	rescheduleTimerFlag.clear();

	stopThread = 0;
	try {
		for(;;) {
			try {
				if (DEBUG_STOP_AT_NOT_RUNNING) {
					if (!running) ERROR();
				}
				Interpreter::execute();
			} catch(RequestReschedule& e) {
				// Only ERROR_RequestReschedule throws RequestReschedule.
				// ERROR_RequestReschedule is called from Reschedule() and ProcessorThread::checkRequestReschedule().
				// In above both case, RequestReschedule will thrown while interrupt is enabled.
				// Also above both case, call is from ProcessorThread
				PERF_COUNT(processor, requestReschedule)
				//logger.debug("Reschedule %-20s  %8d", e.func, rescheduleCount);
				for(;;) {
					// break if OP_STOPEMULATOR is called
					if (stopThread) goto exitLoop;

					// If not running, wait interrupts or timeouts
					if (!running) {
						//logger.debug("waitRunning START");
						std::unique_lock<std::mutex> locker(mutexRequestReschedule);
						for(;;) {
							cvRunning.wait_for(locker, Util::ONE_SECOND);
							if (stopThread) goto exitLoop;
							if (rescheduleInterruptFlag) break;
							if (rescheduleTimerFlag) break;
							//logger.debug("waitRunning WAITING");
						}
						//logger.debug("waitRunning FINISH");
					} else {
						PERF_COUNT(processor, running)
					}
					// Do reschedule.
					{
						//logger.debug("reschedule START");
						bool needReschedule = false;
						if (rescheduleInterruptFlag) {
							PERF_COUNT(processor, interruptFlag)
							//logger.debug("reschedule INTERRUPT");
							// process interrupt
							if (Interrupt()) {
								PERF_COUNT(processor, interrupt)
								needReschedule = true;
							}
						}
						if (rescheduleTimerFlag) {
							PERF_COUNT(processor, timerFlag)
							//logger.debug("reschedule TIMER");
							// process timeout
							if (TimerThread::processTimeout()) {
								PERF_COUNT(processor, timer)
								needReschedule = true;
							}
						}
						if (needReschedule) {
							PERF_COUNT(processor, needReschedule)
							Reschedule(1);
						}
						rescheduleInterruptFlag.clear();
						rescheduleTimerFlag.clear();
						//logger.debug("reschedule FINISH");
					}
					// It still not running, continue loop again
					if (!running) continue;
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
	// stop relevant thread
	AgentNetwork::ReceiveThread::stop();
	AgentNetwork::TransmitThread::stop();
	AgentDisk::IOThread::stop();
	TimerThread::stop();
	InterruptThread::stop();

	logger.info("ProcessorThread::run STOP");
}
void ProcessorThread::requestRescheduleTimer() {
	rescheduleTimerFlag.set();
	if (!running) {
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		cvRunning.notify_one();
	}
}
void ProcessorThread::requestRescheduleInterrupt() {
	rescheduleInterruptFlag.set();
	if (!running) {
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		cvRunning.notify_one();
	}
}


std::set<CARD16> ProcessorThread::stopAtMPSet;

void ProcessorThread::stopAtMP(CARD16 mp) {
	stopAtMPSet.insert(mp);
	logger.info("stopAtMP %4d", mp);
}
void ProcessorThread::mp_observer(CARD16 mp) {
	if (stopAtMPSet.contains(mp)) {
		logger.info("stop at MP %4d", mp);
		stop();
	}
}
