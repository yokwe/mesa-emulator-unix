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

#include <chrono>
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "../util/Util.h"
#include "Constant.h"
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
#include "../util/watchdog.h"


namespace processor_thread {

bool                    stopThread;
std::condition_variable cvRunning;
std::mutex              mutexRequestReschedule;
std::mutex              mutexFlags;
bool                    rescheduleInterruptFlag;
bool                    rescheduleTimerFlag;
std::set<CARD16>        stopAtMPSet;

static uint64_t         time_0900;
static uint64_t         time_8000;

void stop() {
	logger.info("processor_thread::stop");
	TRACE_REC_(processor, stop)
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
	if (mp ==  900) time_0900 = Util::getMilliSecondsSinceEpoch();
	if (mp == 8000) time_8000 = Util::getMilliSecondsSinceEpoch();
}

std::string getBootTime() {
	if (time_0900 == 0 || time_8000 == 0) return "No boot";

	std::string bootAt;
	{
		auto microsecond = time_0900 % 1000;
		time_t time = time_0900 / 1'000;
		struct tm tm;
		localtime_r(&time, &tm);

		bootAt = std_sprintf("%d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, microsecond);
	}
	std::string bootDuration;
	{
		auto time = time_8000 - time_0900;
		auto seconds = time / 1'000;
		auto milliSeconds = time % 1'000;
		bootDuration = std_sprintf("%d.%03d", seconds, milliSeconds);
	}
	auto ret = std_sprintf("Boot started at %s  It took %s seconds", bootAt, bootDuration);
	return ret;
}
std::string getElapsedTime() {
	if (time_0900 == 0) return "No boot";
	auto time = Util::getMilliSecondsSinceEpoch() - time_0900;
	auto seconds = time / 1'000;
	auto minutes = seconds / 60;
	auto hours = minutes / 60;

	seconds %= 60;
	minutes %= 60;

	std::string timeString;
	if (minutes == 0) timeString = std_sprintf("%d seconds", seconds);
	else if (hours == 0) timeString = std_sprintf("%d:%02d", minutes, seconds);
	else timeString =  std_sprintf("%d:%02d:%02d", hours, minutes, seconds);

	return std_sprintf("Elaples time is %s", timeString);
}

void requestRescheduleTimer() {
	PERF_COUNT(processor, requestRescheduleTimer_ENTER)
	TRACE_REC_(processor, requestRescheduleTimer_ENTER)
	{
		TRACE_REC_(processor, mutexFlags_locking)
		std::unique_lock<std::mutex> locker(mutexFlags);
		TRACE_REC_(processor, mutexFlags_locked)
    	rescheduleTimerFlag = true;
	}
	if (!running) {
		TRACE_REC_(processor, mutexRequestReschedule_locking)
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		TRACE_REC_(processor, mutexRequestReschedule_locked)
		TRACE_REC_(processor, cvRunning_notifying)
		cvRunning.notify_one();
		TRACE_REC_(processor, cvRunning_notified)
	}
	PERF_COUNT(processor, requestRescheduleTimer_EXIT)
	TRACE_REC_(processor, requestRescheduleTimer_EXIT)
}
void requestRescheduleInterrupt() {
	PERF_COUNT(processor, requestRescheduleInterrupt_ENTER)
	TRACE_REC_(processor, requestRescheduleInterrupt_ENTER)
	{
		TRACE_REC_(processor, mutexFlags_locking)
		std::unique_lock<std::mutex> locker(mutexFlags);
		TRACE_REC_(processor, mutexFlags_locked)
		rescheduleInterruptFlag = true;
	}
	if (!running) {
		TRACE_REC_(processor, mutexRequestReschedule_locking)
		std::unique_lock<std::mutex> locker(mutexRequestReschedule);
		TRACE_REC_(processor, mutexRequestReschedule_locked)
		TRACE_REC_(processor, cvRunning_notifying)
		cvRunning.notify_one();
		TRACE_REC_(processor, cvRunning_notified)
	}
	PERF_COUNT(processor, requestRescheduleInterrupt_EXIT)
	TRACE_REC_(processor, requestRescheduleInterrupt_EXIT)
}

void checkRequestReschedule() {
    PERF_COUNT(processor, checkRequestReschedule_ENTER)
//    TRACE_REC_(processor, checkRequestReschedule_ENTER)
    if (InterruptsEnabled() && (rescheduleTimerFlag || rescheduleInterruptFlag)) {
        PERF_COUNT(processor, throw_RequestReschedule)
		TRACE_REC_(processor, throw_RequestReschedule)
        ERROR_RequestReschedule();
	}
    // if stopThread is true, throw RequestReschedule
    if (stopThread) {
        ERROR_RequestReschedule();
    }
    PERF_COUNT(processor, checkRequestReschedule_EXIT)
//    TRACE_REC_(processor, checkRequestReschedule_EXIT)
}

void watchdogAction() {
	logger.info("watchdogAction_ENTER");
	TRACE_REC_(processor, watchdogAction_ENTER)
	// stop processor thread
	stop();
	std::this_thread::sleep_for(Util::ONE_SECOND);
	std::this_thread::sleep_for(Util::ONE_SECOND);
	TRACE_REC_(processor, trace_dump)
    trace::dump();
	variable::dump();
	perf::dump();
	logger.info("watchdogAction_EXIT");
	TRACE_REC_(processor, watchdogAction_EXIT)
}

void run() {
	logger.info("processor_thread::run START");
	stopThread              = false;
	rescheduleInterruptFlag = false;
	rescheduleTimerFlag     = false;
	time_0900               = 0;
	time_8000               = 0;

	TaggedControlLink bootLink = {SD + OFFSET_SD(sBoot)};
	XFER(bootLink.u, 0, XferType::call, 0);
	logger.info("bootLink  data  %04X   tag  %d", bootLink.data + 0, bootLink.tag + 0);
	logger.info("GFI = %04X  CB  = %08X  GF  = %08X", GFI, CB, GF);
	logger.info("LF  = %04X  PC  = %04X      MDS = %08X", LF, PC, MDS);

	watchdog::Watchdog watchdog("processor", std::chrono::milliseconds(cTick * 2), watchdogAction);
	watchdog::insert(&watchdog);

	running.timeStart();
	try {
		for(;;) {
			try {
				if (DEBUG_STOP_AT_NOT_RUNNING) {
					if (!running) ERROR();
				}
//				TRACE_REC_(processor, run)
				Execute();
//				TRACE_REC_(processor, run)
			} catch(RequestReschedule& e) {
				TRACE_REC_(processor, catch_RequestReschedule)
				watchdog.update();
				// Only ERROR_RequestReschedule throws RequestReschedule.
				// ERROR_RequestReschedule is called from Reschedule() and processor::checkRequestReschedule().
				// In above both case, RequestReschedule will thrown while interrupt is enabled.
				// Also above both case, call is from ProcessorThread
				PERF_COUNT(processor, requestReschedule_ENTER)
				//logger.debug("Reschedule %-20s  %8d", e.func, rescheduleCount);
				for(;;) {
					TRACE_REC_(processor, for_loop_start)
					// break if OP_STOPEMULATOR is called
					if (stopThread) goto exitLoop;

					// If not running, wait interrupts or timeouts
					if (running) {
						PERF_COUNT(processor, running_A_YES)
						TRACE_REC_(processor, running_A_YES)
					} else {
						PERF_COUNT(processor, running_A_NO)
						TRACE_REC_(processor, running_A_NO)
						//logger.debug("waitRunning START");
						for(;;) {
							TRACE_REC_(processor, mutexRequestReschedule_locking)
							std::unique_lock<std::mutex> locker(mutexRequestReschedule);
							TRACE_REC_(processor, mutexRequestReschedule_locked)
							TRACE_REC_(processor, cvRunning_wait_for_calling)
							cvRunning.wait_for(locker, Util::ONE_SECOND);
							TRACE_REC_(processor, cvRunning_wait_for_called)
							if (stopThread) goto exitLoop;
							if (rescheduleInterruptFlag) break;
							if (rescheduleTimerFlag)     break;
							//logger.debug("waitRunning WAITING");
						}
						TRACE_REC_(processor, exit_inner_for_loop)
						//logger.debug("waitRunning FINISH");
					}
					// Do reschedule.
					bool interruptFlag;
					bool timerFlag;
					{
						TRACE_REC_(processor, mutexFlags_locking)
						std::unique_lock<std::mutex> locker(mutexFlags);
						TRACE_REC_(processor, mutexFlags_locked)
						interruptFlag = rescheduleInterruptFlag;
						timerFlag     = rescheduleTimerFlag;
						rescheduleInterruptFlag = false;
						rescheduleTimerFlag     = false;
					}

					//logger.debug("reschedule START");
					bool needReschedule = false;
					if (interruptFlag) {
						PERF_COUNT(processor, interruptFlag_YES)
						TRACE_REC_(processor, interruptFlag_YES)
						//logger.debug("reschedule INTERRUPT");
						// process interrupt
						TRACE_REC_(processor, Interrupt_calling)
						auto result = Interrupt();
						TRACE_REC_(processor, Interrupt_called)
						if (result) {
							PERF_COUNT(processor, interrupt)
							needReschedule = true;
						}
					} else {
						PERF_COUNT(processor, interruptFlag_NO)
						TRACE_REC_(processor, interruptFlag_NO)
					}
					if (timerFlag) {
						PERF_COUNT(processor, timerFlag_YES)
						TRACE_REC_(processor, timerFlag_YES)
						//logger.debug("reschedule TIMER");
						// process timeout
						TRACE_REC_(processor, processTimeout_calling)
						auto result = timer_thread::processTimeout();
						TRACE_REC_(processor, processTimeout_called)
						if (result) {
							PERF_COUNT(processor, timer)
							needReschedule = true;
						}
					} else {
						PERF_COUNT(processor, timerFlag_NO)
						TRACE_REC_(processor, timerFlag_NO)
					}
					if (needReschedule) {
						PERF_COUNT(processor, needReschedule_YES)
						TRACE_REC_(processor, Reschedule_calling)
						Reschedule(1);
						TRACE_REC_(processor, Reschedule_called)
					} else {
						PERF_COUNT(processor, needReschedule_NO)
					}
					// It still not running, continue loop again
					if (running) {
						PERF_COUNT(processor, running_B_YES)
						TRACE_REC_(processor, running_B_YES)
						break;
					} else {
						PERF_COUNT(processor, running_B_NO)
						TRACE_REC_(processor, running_B_NO)
					}
				}
				TRACE_REC_(processor, requestReschedule_EXIT)
				PERF_COUNT(processor, requestReschedule_EXIT)
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
	TRACE_REC_(processor, exitLoop)
	running.timeStop();
	watchdog::remove(&watchdog);

	// stop relevant thread
	AgentNetwork::ReceiveThread::stop();
	AgentNetwork::TransmitThread::stop();
	AgentDisk::IOThread::stop();
	timer_thread::stop();
	interrupt_thread::stop();

	logger.info("processor_thread::run STOP");
}

}