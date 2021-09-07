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
// SPPQueue.h
//

#pragma once

#include <functional>

#include <QtConcurrent/QtConcurrent>

#include "SPPListener.h"

class SPPQueue : public SPPListener {
public:
	class State {
	public:
		const char* name;

		quint64 time;

		quint64 remoteHost;
		quint16 remoteSocket;
		quint16 remoteID;

		quint16 localSocket;
		quint16 localID;

		quint16 sst;

		// Packets with the Attention bit set must have only one byte of data.

		// The Sequence Number counts packets sent on the connection.
		// The first packet is assigned number zero, and the count proceeds from there.
		// If the count overflows the 16-bit field, the overflow is ignored and the count proceeds from zero again.
		quint16 seq;
		// The Acknowledge Number field specifies the sequence number of the first packet which has not yet been seen traveling in the reverse direction.
		// Acknowledge Number indicates the sequence number of the next expected packet.
		quint16 ack;
		// The Allocation Number specifies the sequence number up to and including which packets will be accepted from the other end.
		// One plus the difference between the Allocation Number and the Acknowledge Number indicates the number of packets that may be outstanding in the reverse direction
		quint16 alloc;

		State() : name(nullptr), time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0), sst(0), seq(0), ack(0), alloc(0) {}
		State(const State& that) {
			this->name         = that.name;
			this->time         = that.time;
			this->remoteHost   = that.remoteHost;
			this->remoteSocket = that.remoteSocket;
			this->remoteID     = that.remoteID;
			this->localSocket  = that.localSocket;
			this->localID      = that.localID;
			this->sst          = that.sst;
			this->seq          = that.seq;
			this->ack          = that.ack;
			this->alloc        = that.alloc;
		}
		State& operator = (const State& that) {
			this->name         = that.name;
			this->time         = that.time;
			this->remoteHost   = that.remoteHost;
			this->remoteSocket = that.remoteSocket;
			this->remoteID     = that.remoteID;
			this->localSocket  = that.localSocket;
			this->localID      = that.localID;
			this->sst          = that.sst;
			this->seq          = that.seq;
			this->ack          = that.ack;
			this->alloc        = that.alloc;
			return *this;
		}
	};

	SPPQueue(const char* name, quint16 socket);
	virtual ~SPPQueue() {}

	SPPQueue(const SPPQueue& that);

	void init();
	void start();
	void stop();

	void handle(const XNS::Data& data, const XNS::SPP& spp);

	// clone method for SPPQueue
	virtual SPPQueue* clone() = 0;
	// create copy constructor and call it in clone()
	// return new SPPServerImpl(*this);

	void state(const State& state_) {
		myState = state_;
	}

protected:
	// if getData returns true, data and spp are assigned
	// if getDAta returns false, data and spp are NOT assigned
	bool                    getData(XNS::Data* data, XNS::SPP* spp);
	bool                    stopRun();
	XNS::Config*            getConfig();
	XNS::Context*           getContext();
	XNS::Server::Listeners* getListeners();

	class FunctionTable {
	public:
		std::function<bool(XNS::Data*, XNS::SPP*)>   getData;
		std::function<bool(void)>                    stopRun;
		std::function<XNS::Config*(void)>            getConfig;
		std::function<XNS::Context*(void)>           getContext;
		std::function<XNS::Server::Listeners*(void)> getListeners;
	};
	FunctionTable functionTable;

	virtual void run(FunctionTable functionTable) = 0;

	State myState;

private:
	class MyData {
	public:
		XNS::Data data;
		XNS::SPP  spp;
	};

	bool           stopFuture;
	QFuture<void>  future;

	QList<MyData>  dataList;
	QMutex         dataListMutex;
	QWaitCondition dataListCV;
};


class SPPQueueServer : public SPPListener {
public:
	SPPQueueServer(SPPQueue* impl) : SPPListener(impl->name(), impl->socket()), myImpl(impl) {}

	void handle(const XNS::Data& data, const XNS::SPP& spp);

private:
	SPPQueue* myImpl;
};
