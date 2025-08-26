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

#include "MesaBasic.h"
#include "Type.h"


template<typename T>
class VariableBase {
public:
    class Observer {
        std::vector<std::function<void(T)>> observerList;
    public:
        void add(std::function<void(T)> observer) {
            observerList.push_back(observer);
        }
        void notify(T value) {
            for(auto e: observerList) e(value);
        }
    };
private:
    Observer setObserver;
    Observer getObserver;
protected:
    virtual void setValue(T newValue) = 0;
    virtual T getValue() = 0;

    void addSetObserver(const std::function<void(T)>& newObserver) {
        setObserver.add((newObserver));
    }
    void addGetObserver(const std::function<void(T)>& newObserver) {
        getObserver.add((newObserver));
    }
    virtual void addObserver() = 0;

    virtual ~VariableBase() {}

public:

    T operator=(const T& newValue) {
        T temp(newValue);
        setValue(temp);
        setObserver.notify(temp);
        return temp;
    }
    operator T() {
        T temp = getValue();
        getObserver.notify(temp);
        return temp;
    }
 };

template<typename T>
class VariableSimple : public VariableBase<T> {
private:
    T storage;
protected:
    void setValue(T newValue) override {
        storage = newValue;
    }
    T getValue() override {
        return storage;
    }
    VariableSimple(T newValue) {
        setValue(newValue);
    }
public:
    T operator=(const T newValue) {
        return VariableBase<T>::operator=(newValue);
    }
};
template<typename T>
class VariableAtomic : public VariableBase<T> {
private:
    std::atomic<T> atomicStorage;
protected:
    void setValue(T newValue) override {
        atomicStorage.store(newValue);
    }
    T getValue() override {
        return atomicStorage.load();
    }
    VariableAtomic(T newValue) {
        setValue(newValue);
    }
public:
    T operator=(const T newValue) {
        return VariableBase<T>::operator=(newValue);
    }
};


class VariableMP final : public VariableSimple<CARD16> {
    void addObserver() override;
public:
    VariableMP() : VariableSimple(0) {
        addObserver();
    }

    void clear() {
        VariableSimple<CARD16>::setValue(0);
    }
    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;

    CARD16 operator=(const CARD16 newValue) {
        return VariableBase<CARD16>::operator=(newValue);
    }
};


class VariableWDC  {
    std::atomic<CARD16> value;
public:
    VariableWDC() {
        value.store(0);
    }

    // prohibit assignment from int
    CARD16 operator=(const int newValue) = delete;

    CARD16 operator=(const CARD16 newValue) {
        value.store(newValue);
        return newValue;
    }
    operator CARD16() {
        return value.load();
    }

    void enable() {
        value.fetch_sub(1);
    }
    void disable() {
        value.fetch_add(1);
    }
    bool isEnabled() {
        return value.load() == 0;
    }
};

// 3.3.2 Evaluation Stack
extern CARD16 stack[StackDepth];
extern CARD16 SP;

// 3.3.3 Data and Status Registers
extern CARD16 PID[4]; // Processor ID

//extern CARD16 MP;     // Maintenance Panel
extern VariableMP MP;

//extern CARD32 IT;     // Interval Timer
//extern CARD16 WM;     // Wakeup mask register - 10.4.4
//extern CARD16 WP;     // Wakeup pending register - 10.4.4.1
//extern CARD16 WDC;    // Wakeup disable counter - 10.4.4.3
extern VariableWDC WDC;


//extern CARD16 PTC;    // Process timeout counter - 10.4.5
extern CARD16 XTS;    // Xfer trap status - 9.5.5

// 3.3.1 Control Registers
extern CARD16            PSB; // PsbIndex - 10.1.1
//extern MdsHandle         MDS;
extern LocalFrameHandle  LF;  // POINTER TO LocalVariables
extern GlobalFrameHandle GF;  // LONG POINTER TO GlobalVarables
extern CARD32            CB;  // LONG POINTER TO CodeSegment
extern CARD16            PC;
extern GFTHandle         GFI;

// 4.5 Instruction Execution
extern CARD8 breakByte;
extern CARD16 savedPC;
extern CARD16 savedSP;

// 10.4.1 Scheduler
//extern int running;

// 10.4.5 Timeouts
// TimeOutInterval:LONG CARDINAL;
// One tick = 40 milliseconds
//const LONG_CARDINAL TimeOutInterval = 40 * 1000;

// time: LONG CARDINAL
// Due to name conflict with time, rename to time_CheckForTimeouts
//extern LONG_CARDINAL lastTimeoutTime;
