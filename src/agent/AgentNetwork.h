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
// AgentNetwork.h
//

#pragma once

#include <mutex>
#include <deque>

#include "../util/net.h"
#include "../util/ThreadQueue.h"

#include "Agent.h"

class AgentNetwork : public Agent {
public:
    using EthernetIOCBType = EthernetIOFaceGuam::EthernetIOCBType;
    using EthernetFCBType  = EthernetIOFaceGuam::EthernetFCBType;

	struct Item {
		CARD16            interruptSelector;
		EthernetIOCBType* iocb;

		Item(CARD16 interruptSelector_, EthernetIOCBType* iocb_) :
			interruptSelector(interruptSelector_), iocb(iocb_) {}
		// use default implementation
		Item(const Item& that)            = default;
		Item& operator=(const Item& that) = default;	
	};

	class TransmitThread : public thread_queue::ThreadQueueProcessor<Item> {
		net::Driver* driver;
	public:
		static void stop() {
			thread_queue::ThreadQueueProcessor<Item>::stop();
		}

		TransmitThread() : driver(0) {}

		void set(net::Driver* driver_) {
			driver = driver_;
		}

		void process(const Item& data);
	};

	class ReceiveThread {
		static inline bool stopThread;
		std::mutex         mutex;
		std::deque<Item>   queue;
		net::Driver*       driver;
	public:
		static void stop() {
			stopThread = true;
		}

		ReceiveThread() : driver(0) {}

		void set(net::Driver* driver_) {
			driver = driver_;
		}

		void push(const Item& item);
		void run();
		void process(const Item& item, const std::span<uint8_t>& span);
	};

	ReceiveThread  receiveThread;
	TransmitThread transmitThread;

	static const inline auto index_ = GuamInputOutput::AgentDeviceIndex::network;
	static const inline auto name_ = "Network";
	static const inline auto fcbSize_ = SIZE(EthernetFCBType);
	AgentNetwork() : Agent(index_, name_, fcbSize_), fcb(0), driver(0) {}

	void Initialize();
	void Call();

	void poll();

	void setDriver(net::Driver* driver_) {
		driver = driver_;
	}

private:
	EthernetFCBType* fcb;
	net::Driver*     driver;
};
