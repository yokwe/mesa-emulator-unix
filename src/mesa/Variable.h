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
// Variable.h
//

#pragma once

#include <vector>
#include <functional>
#include <atomic>

#include "Constant.h"
#include "Function.h"
#include "MesaBasic.h"
#include "Type.h"

#include "../util/Util.h"
#include "../util/Perf.h"


namespace variable {
    extern void initialize();
}


#define STACK_ERROR() { \
	logger.fatal("STACK_ERROR  %s -- %5d %s", __FUNCTION__, __LINE__, __FILE__); \
	StackError(); \
}
#define MINIMAL_STACK() { \
    if (SP != 0) { \
        logger.fatal("MINIMAL_STACK  %s -- %5d %s", __FUNCTION__, __LINE__, __FILE__); \
        STACK_ERROR(); \
    } \
}


class VariableMP {
    CARD16 storage;

    std::vector<std::function<void(CARD16)>> observerList;
    void initialize();
public:
    VariableMP() {
        storage = 0;
        initialize();
    }

    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;
    CARD16 operator=(const CARD16 newValue) {
        storage = newValue;
        for(auto observer: observerList) observer(newValue);
        return newValue;
    }
    operator CARD16() {
        return storage;
    }

    void clear() {
        storage = 0;
    }
};


class VariableWDC {
    std::atomic<CARD16> storage;
public:
    VariableWDC() {
        storage.store(0);
    }

    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;
    CARD16 operator=(const CARD16 newValue) {
        storage.store(newValue);
        return newValue;
    }
    operator CARD16() {
        return storage.load();
    }

    void enable() {
        if (storage.load() == 0) InterruptError();
        storage.fetch_sub(1);
    }
    void disable() {
        if (storage.load() == cWDC) InterruptError();
        storage.fetch_add(1);
    }
    bool enabled() {
        return storage.load() == 0;
    }
};


class VariableWP {
    std::atomic<CARD16> storage;
public:
    VariableWP() {
        storage.store(0);
    }

    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;
    CARD16 operator=(const CARD16 newValue) {
        storage.store(newValue);
        return newValue;
    }
    operator CARD16() {
        return storage.load();
    }

    CARD16 exchange(CARD16 value) {
        return storage.exchange(value);
    }
    CARD16 fetch_or(CARD16 value) {
        return storage.fetch_or(value);
    }
    bool pending() {
        return storage.load() != 0;
    }
};


// IT use one microsecond time
class VariableIT {
public:
    static const CARD32 MicrosecondsPerHundredPulses = 100;
    static const CARD16 MillisecondsPerTick          = cTick;
    operator CARD32() {
        return (CARD32)Util::getMicroSecondsFromEpoch();
    }
};


class VariableRunning {
    bool storage;
public:
    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;
    CARD16 operator=(const bool newValue) {
        storage = newValue;
        if (newValue) {
            PERF_COUNT(running, start)
        } else {
            PERF_COUNT(running, stop)
        }

        return newValue;
    }
    operator bool() {
        return storage;
    }

    void clear() {
        storage = false;
    }
};


class VariableAtomicFlag {
    std::atomic_flag storage;
public:
    VariableAtomicFlag() {
        storage.clear();
    }

    void set() {
        storage.test_and_set();
    }
    void clear() {
        storage.clear();
    }
    operator bool() {
        return storage.test();
    }
};
class VariableAtomicBool {
    std::atomic<bool> storage;
public:
    VariableAtomicBool() {
        storage.store(false);
    }

    void set() {
        storage.store(true);
    }
    void clear() {
        storage.store(false);
    }
    operator bool() {
        return storage.load();
    }
};


extern CARD16 SP;
class VariableStack {
    CARD16 storage[StackDepth];
public:
    void recover() {
        if (StackDepth == SP) StackError();
        SP++;
    }
    void discard() {
        if (SP == 0) StackError();
        SP--;
    }
    void clear() {
        for(int i = 0; i < StackDepth; i++) {
            storage[i] = 0;
        }
        SP = 0;
    }
    void push(CARD16 value) {
        if (StackDepth == SP) StackError();
	    storage[SP++] = value;
    }
    CARD16 pop() {
        if (SP == 0) StackError();
	    return storage[--SP];
    }
    void pushLong(CARD32 value) {
        if ((StackDepth - 1) <= SP) StackError();
        storage[SP++] = (CARD16)value;
        storage[SP++] = (CARD16)(value >> WordSize);
    }
    CARD32 popLong() {
        if (SP <= 1) StackError();
        CARD32 ret = storage[--SP] << WordSize;
        return ret | storage[--SP];
    }

    CARD16 operator[](int n) const {
        if (n < 0 || StackDepth <= n) StackError();
        return storage[n];
    }
    CARD16& operator[](int n) {
        if (n < 0 || StackDepth <= n) StackError();
        return storage[n];
    }
};


class VariableMDS {
    CARD32 storage;
public:
    VariableMDS() : storage(0) {}

    CARD32 operator=(const int newValue) = delete;
    CARD32 operator=(const CARD32 newValue) {
        storage = newValue;
        return newValue;
    }
    operator CARD32() {
        return storage;
    }
    CARD32 lengthenPointer(int pointer) = delete;
    CARD32 lengthenPointer(CARD16 pointer) {
        return storage + pointer;
    }
};


class VariableCB {
    CARD32 storage;
public:
    VariableCB() : storage(0) {}

    CARD32 operator=(const int newValue) = delete;
    CARD32 operator=(const CARD32 newValue) {
        storage = newValue;
        return newValue;
    }
    operator CARD32() {
        return storage;
    }
};


class VariableLF {
    CARD16 storage;
public:
    VariableLF() : storage(0) {}

    CARD16 operator=(const int newValue) = delete;
    CARD16 operator=(const CARD16 newValue) {
        storage = newValue;
        return newValue;
    }
    operator CARD16() {
        return storage;
    }
};


class VariableGF {
    CARD32 storage;
public:
    VariableGF() : storage(0) {}

    CARD32 operator=(const int newValue) = delete;
    CARD32 operator=(const CARD32 newValue) {
        storage = newValue;
        return newValue;
    }
    operator CARD32() {
        return storage;
    }
};


// 3.3.2 Evaluation Stack
//extern CARD16 stack[StackDepth];
extern VariableStack stack;
extern CARD16 SP;

inline void Recover() {
    stack.recover();
}
inline void Discard() {
    stack.discard();
}
inline void Push(CARD16 value) {
    stack.push(value);
};
inline CARD16 Pop() {
    return stack.pop();
}
inline void PushLong(CARD32 value) {
    stack.pushLong(value);
}
inline CARD32 PopLong() {
    return stack.popLong();
}

// 3.3.3 Data and Status Registers
extern CARD16 PID[4]; // Processor ID

//extern CARD16 MP;     // Maintenance Panel
extern VariableMP MP;

//extern CARD32 IT;     // Interval Timer
extern VariableIT IT;
static const CARD32 MicrosecondsPerHundredPulses = VariableIT::MicrosecondsPerHundredPulses;
static const CARD16 MillisecondsPerTick = VariableIT::MillisecondsPerTick;

//extern CARD16 WM;     // Wakeup mask register - 10.4.4
//extern CARD16 WP;     // Wakeup pending register - 10.4.4.1
extern VariableWP WP;

//extern CARD16 WDC;    // Wakeup disable counter - 10.4.4.3
extern VariableWDC WDC;
inline bool InterruptsEnabled() {
    return WDC.enabled();
}
inline void DisableInterrupt() {
    WDC.disable();
}
inline void EnableInterrupts() {
    WDC.enable();
}

extern CARD16 PTC;    // Process timeout counter - 10.4.5
extern CARD16 XTS;    // Xfer trap status - 9.5.5

// 3.3.1 Control Registers
extern CARD16            PSB; // PsbIndex - 10.1.1

//extern MdsHandle       MDS;
extern VariableMDS       MDS;
inline CARD32 LengthenPointer(CARD16 pointer) {
    return MDS.lengthenPointer(pointer);
}

//extern LocalFrameHandle  LF;  // POINTER TO LocalVariables
extern VariableLF        LF;

//extern GlobalFrameHandle GF;  // LONG POINTER TO GlobalVarables
extern VariableGF        GF;

//extern CARD32            CB;  // LONG POINTER TO CodeSegment
extern VariableCB        CB;

extern CARD16            PC;
extern GFTHandle         GFI;

// 4.5 Instruction Execution
extern CARD8  breakByte;
extern CARD16 savedPC;
extern CARD16 savedSP;

// 10.4.1 Scheduler
extern VariableRunning running;
