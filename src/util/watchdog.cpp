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
// watchdog.cpp
//

#include <cstring>
#include <mutex>
#include <deque>
#include <thread>


#include "Util.h"
static const Logger logger(__FILE__);

#include "watchdog.h"

namespace watchdog {

std::mutex             mutex;
std::deque<Watchdog*> all;

bool enableScan = false;

bool scanThreadRunning = false;
std::thread scanThread;

static std::string toStringLocalTime(const std::chrono::system_clock::time_point time) {
    time_t temp = std::chrono::system_clock::to_time_t(time);
	auto microsecond = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1'000'000;

    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%d-%02d-%02d %02d:%02d:%02d.%06d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, microsecond);
}

void run() {
    for(;;) {
        std::this_thread::sleep_for(Util::ONE_SECOND);
        if (enableScan) {
            std::unique_lock<std::mutex> lock(mutex);
            auto time = std::chrono::system_clock::now();
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();

            for(auto e: all) {
                auto elapsedTime = now - e->updateTime;
                if (e->threshold < elapsedTime) {
                    logger.info("callAction  %s  %s  %d", toStringLocalTime(time), e->name, elapsedTime);
                    e->action();
                }
            }
        }
    }
}
void start() {
    if (!scanThreadRunning) {
        scanThread = std::thread(run);
        scanThread.detach();
        scanThreadRunning = true;
        logger.info("scanThread started");
    }
}
void enable() {
    std::unique_lock<std::mutex> lock(mutex);
    enableScan = true;
    for(auto e: all) e->update();
}
void disable() {
    std::unique_lock<std::mutex> lock(mutex);
    enableScan = false;
}

void insert(Watchdog* watchdog) {
    std::unique_lock<std::mutex> lock(mutex);
    // sanity chedk
    for(auto e: all) {
        if (e == watchdog) {
            logger.error("Already exist in all");
            logger.error("old  %s  %p", e->name, e);
            logger.error("new  %s  %p", watchdog->name, watchdog);
            ERROR();
        }
    }
    all.push_back(watchdog);
}
void remove(Watchdog* watchdog) {
    std::unique_lock<std::mutex> lock(mutex);
    int count = 0;
    for(auto i = all.begin(); i != all.end();) {
        if (*i == watchdog) {
            i = all.erase(i);
            count++;
        } else {
            i++;
        }
    }
    if (count != 1) {
        logger.error("Not found in all");
        logger.error("watchdog  %s  %p", watchdog->name, watchdog);
        ERROR();
    }
}

}