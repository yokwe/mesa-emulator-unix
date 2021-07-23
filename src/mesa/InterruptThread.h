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
// InterruptThread.h
//

#ifndef INTERRUPT_THREAD_H__
#define INTERRUPT_THREAD_H__

#include "MesaBasic.h"
#include "Constant.h"

#include <QtCore>

class InterruptThread : public QRunnable {
public:
	static const QThread::Priority PRIORITY = QThread::NormalPriority;

	// Wait interval in milliseconds for QWaitCondition::wait
	static const int WAIT_INTERVAL = 1000;

	static CARD16 getWP();
	static void   setWP(CARD16 newValue);

	// WDC is changed from processor thread only, so there is no race condition.
	static inline CARD16 getWDC() {
		return WDC;
	}
	static inline void   setWDC(CARD16 newValue) {
		WDC = newValue;
	}
	static inline void enable() {
		setWDC(WDC - 1);
	}
	static inline void disable() {
		setWDC(WDC + 1);
	}
	static inline int isEnabled() {
		return WDC == 0;
	}
//	static int  isPending();
	static void notifyInterrupt(CARD16 interruptSelector);
	static void stop();

	void run();
private:
	static CARD16         WP;
	static CARD16         WDC;
	static QMutex         mutexWP;
	static QWaitCondition cvWP;
	static int            stopThread;

	static int interruptCount;
	static int notifyCount;
	static int notifyWakeupCount;
};

#endif
