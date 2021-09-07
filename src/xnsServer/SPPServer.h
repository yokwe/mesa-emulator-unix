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
// SPPServer.h
//

#pragma once

#include <functional>

#include <QtConcurrent/QtConcurrent>

#include "../xnsServerImpl/PilotStream.h"
#include "SPPQueue.h"

namespace XNS::Server {

	class SPPServerImpl : public SPPListener {
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

			// FIXME
			//   how to retransmit data?
			//   how to remove duplicate of received data?
			//   how to increase my seq number?
			//

			// If ack of receiving packet, means expecing sequence number

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

		SPPServerImpl() : SPPListener() {}
		virtual ~SPPServerImpl() {}

		SPPServerImpl(const SPPServerImpl& that) : SPPListener(that) {
			this->myState = that.myState;
		}
		SPPServerImpl& operator = (const SPPServerImpl& that) {
			SPPListener::operator =(that);
			this->myState = that.myState;
			return *this;
		}

		void state(const State& state_) {
			myState = state_;
		}

		SPPServerImpl(const char* name, quint16 socket) : SPPListener(name, socket) {}
		SPPServerImpl(const State& state_) : SPPListener(state_.name, state_.localSocket), myState(state_) {}

		void handle(const XNS::Data& data, const XNS::SPP& spp);

		class FunctionTable {
		public:
			std::function<bool(void)>             stopRun;
			std::function<PilotStream::Stream*()> getPilotStream;
		};

		virtual void           run(FunctionTable functionTable) = 0;

		// clone method for SPPServer
		virtual SPPServerImpl* clone() = 0;
		// create copy constructor and call it in clone()
		// return new SPPServerImpl(*this);

	protected:
		State myState;
	};

	// SPPServer waiting for new connection at server socket
	// If packet is arrived, clone ServerImpl and set new name and socket and add to listeners
	class SPPServer : public SPPListener {
	public:
		SPPServer(SPPServerImpl* impl_) : SPPListener(impl_->name(), impl_->socket()), impl(impl_) {}
		virtual ~SPPServer() {}

		void init();
		void handle(const XNS::Data& data, const XNS::SPP& spp);

	protected:
		SPPServerImpl* impl;
	};


}
