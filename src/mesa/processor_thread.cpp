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
// processor_thread.cpp
//

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>

#include "../util/Util.h"
#include "Constant.h"
#include "Function.h"
static const Logger logger(__FILE__);

#include "../agent/AgentDisk.h"
#include "../agent/AgentNetwork.h"

#include "../opcode/opcode.h"

#include "processor_thread.h"
#include "Variable.h"

#include "../util/Perf.h"
#include "../util/trace.h"
#include "../util/watchdog.h"


namespace processor_thread {

bool                    stopThread;
std::set<CARD16>        stopAtMPSet;

static std::chrono::steady_clock::time_point NO_TIME = std::chrono::steady_clock::time_point::min();
static std::chrono::steady_clock::time_point time_0900 = NO_TIME;
static std::chrono::steady_clock::time_point time_8000 = NO_TIME;

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
	if (mp ==  900) time_0900 = std::chrono::steady_clock::now();
	if (mp == 8000) time_8000 = std::chrono::steady_clock::now();
}

std::string getBootTime() {
	if (time_0900 == NO_TIME || time_8000 == NO_TIME) return "Before boot";

	std::string bootAt;
	{
		auto time_system = to_system_clock(time_0900);
		auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_system.time_since_epoch()).count() % 1000;

		time_t time = std::chrono::system_clock::to_time_t(time_system);
		struct tm tm;
		localtime_r(&time, &tm);

		bootAt = std_sprintf("%d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, milliSeconds);
	}
	std::string bootDuration;
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_8000 - time_0900).count();
		auto seconds = duration / 1'000;
		auto milliSeconds = duration % 1'000;
		bootDuration = std_sprintf("%d.%03d", seconds, milliSeconds);
	}
	auto ret = std_sprintf("Boot started at %s  It took %s seconds", bootAt, bootDuration);
	return ret;
}
std::string getElapsedTime() {
	if (time_0900 == NO_TIME) return "Before boot";
	auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_0900).count();
	auto seconds = milliSeconds / 1'000;
	auto minutes = seconds / 60;
	auto hours = minutes / 60;

	milliSeconds %= 1000;
	seconds %= 60;
	minutes %= 60;

	std::string timeString;
	if (minutes == 0) timeString = std_sprintf("%d.%03d seconds", seconds, milliSeconds);
	else if (hours == 0) timeString = std_sprintf("%d:%02d", minutes, seconds);
	else timeString =  std_sprintf("%d:%02d:%02d", hours, minutes, seconds);

	return std_sprintf("Elaples time is %s", timeString);
}

static void watchdogAction() {
	logger.info("watchdogAction_ENTER");
	TRACE_REC_(processor, watchdogAction_ENTER)
	variable::dump();
	perf::dump();
	variable::dump();
	perf::dump();
    trace::dump();
	// stop processor thread
	stop();
	variable::dump();
	perf::dump();
	logger.info("watchdogAction_EXIT");
	TRACE_REC_(processor, watchdogAction_EXIT)
}

std::mutex              reschuduleMutex;
std::condition_variable rescheduleCV;
std::atomic_bool        timeoutFlag;
std::atomic_bool        interruptFlag;

void run_processor() {
	logger.info("processor_thread::run START");
	stopThread              = false;
	time_0900               = NO_TIME;
	time_8000               = NO_TIME;

	TaggedControlLink bootLink = {SD + OFFSET_SD(sBoot)};
	XFER(bootLink.u, 0, XferType::call, 0);
	logger.info("bootLink  data  %04X   tag  %d", bootLink.data + 0, bootLink.tag + 0);
	logger.info("GFI = %04X  CB  = %08X  GF  = %08X", GFI, CB, GF);
	logger.info("LF  = %04X  PC  = %04X      MDS = %08X", LF, PC, MDS);

	watchdog::Watchdog watchdog("processor", std::chrono::milliseconds(cTick * 2), watchdogAction);
	watchdog::insert(&watchdog);

	interruptFlag = false;
	timeoutFlag = false;

	running.timeStart();
	try {
		for(;;) {
			PERF_COUNT(processor, for_loop)
			if (stopThread) goto exitLoop;
			if (interruptFlag || timeoutFlag) {
				if (InterruptsEnabled()) {
					PERF_COUNT(processor, interruptEnabled_YES)

					bool interruptFlagCopy = interruptFlag.exchange(false);
					bool timeoutFlagCopy = timeoutFlag.exchange(false);

					bool interrupt = false;
					bool timeout   = false;
					if (interruptFlagCopy) {
						PERF_COUNT(processor, interruptFlag)
						if (WP.pending()) interrupt = Interrupt();
					}
					if (timeoutFlagCopy) {
						PERF_COUNT(processor, timeoutFlag)
						PERF_COUNT(processor, updatePTC)
						PTC++;
						if (PTC == 0) PTC++;
						timeout = TimeoutScan();
					}

					if (interrupt || timeout) {
						PERF_COUNT(processor, reschedule_YES)
						watchdog.update();
						Reschedule(true);
					} else {
						PERF_COUNT(processor, reschedule_NO)
					}
					continue;
				} else {
					PERF_COUNT(processor, interruptEnabled_NO)
				}
			}
			if (running) {
				PERF_COUNT(processor, running_YES)
				try {
					Execute();
				} catch (Abort& e) {
					PERF_COUNT(processor, abort)
				}
			} else {
				PERF_COUNT(processor, running_NO)
				std::unique_lock<std::mutex> lock(reschuduleMutex);
				for(;;) {
					rescheduleCV.wait_for(lock, Util::ONE_SECOND);
					if (stopThread) goto exitLoop;
					if (interruptFlag) break;
					if (timeoutFlag) break;
				}
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

	logger.info("processor_thread::run STOP");
}

void run_timer() {
	auto tick = std::chrono::milliseconds(cTick);
	auto time = std::chrono::steady_clock::now();

	for(;;) {
		if (stopThread) break;
		auto nextTime = time + tick;
		std::this_thread::sleep_until(nextTime);
		time = nextTime;
		std::unique_lock<std::mutex> lock(reschuduleMutex);
		PERF_COUNT(processor, timeoutRequest)
		timeoutFlag = true;
		rescheduleCV.notify_one();
	}
}

void notifyInterrupt(CARD16 value) {
	PERF_COUNT(processor, interruptRequest)
    WP |= value;
	std::unique_lock<std::mutex> lock(reschuduleMutex);
	interruptFlag = true;
	rescheduleCV.notify_one();
}

}