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
// CourierListener.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("listen-cour");

#include "../xns/SPP.h"
#include "../xns/Time.h"

#include "../xnsServer/CourierListener.h"

using ByteBuffer::UINT16;
using XNS::Data;
using XNS::Host;
using XNS::IDP;
using XNS::SPP;
using XNS::Socket;

class Key {
public:
	XNS::Host host;
	UINT16    id;

	Key() {}
	Key(const Key& that) {
		this->host = that.host;
		this->id   = that.id;
	}
	Key& operator = (const Key& that) {
		this->host = that.host;
		this->id   = that.id;
		return *this;
	}

	Key(XNS::Host host_, UINT16 id_) : host (host_), id(id_) {}

	bool operator == (const Key& that) const {
		return this->host == that.host && this->id == that.id;
	}
	bool operator < (const Key& that) const {
		if (this->host == that.host) {
			return this->id < that.id;
		} else {
			return this->host < that.host;
		}
	}
};

class State {
public:
	quint64  time;

	Host     hostRemote;
	quint16  idRemote;
	quint16  idLocal;

	SPP::SST sst;     // Sub System Type

	quint16  seq;     // sequence
	quint16  ack;     // acknowledgment
	quint16  alloc;   // allocation

	State() : time(0), idRemote(0), idLocal(0), seq(0), ack(0), alloc(4) {}
	State(const State& that) {
		this->time       = that.time;
		this->hostRemote = that.hostRemote;
		this->idRemote   = that.idRemote;
		this->idLocal    = that.idLocal;
		this->sst        = that.sst;
		this->seq        = that.seq;
		this->ack        = that.ack;
		this->alloc      = that.alloc;
	}
	State& operator = (const State& that) {
		this->time       = that.time;
		this->hostRemote = that.hostRemote;
		this->idRemote   = that.idRemote;
		this->idLocal    = that.idLocal;
		this->sst        = that.sst;
		this->seq        = that.seq;
		this->ack        = that.ack;
		this->alloc      = that.alloc;
		return *this;
	}
};
QMap<Key, State> stateMap;

void CourierListener::run(FunctionTable functionTable) {
	logger.info("CourierListener::run START");

	Data data;
	SPP  spp;

	for(;;) {
		if (functionTable.stopRun()) break;
		bool dataReady = functionTable.getData(&data, &spp);
		if (!dataReady) continue;

		{
			QString timeStamp = QDateTime::fromMSecsSinceEpoch(data.timeStamp).toString("yyyy-MM-dd hh:mm:ss.zzz");
			QString header = QString::asprintf("%s %-18s  %s", TO_CSTRING(timeStamp), TO_CSTRING(data.ethernet.toString()), TO_CSTRING(data.idp.toString()));
			logger.info("%s  SPP   %s  COURIER", TO_CSTRING(header), TO_CSTRING(spp.toString()));
		}

		Key key(data.idp.srcHost, spp.idSrc);
		State state;
		if (stateMap.contains(key)) {
			state = stateMap[key];
		} else {
			// new connection
			// FIXME
			// assign new local id
			// assign new local socket and create listener for new local socket and use it

			state.time       = data.timeStamp;
			state.hostRemote = data.idp.srcHost;
			state.idRemote   = spp.idSrc;
			state.idLocal    = (quint16)(data.timeStamp / 32);
			state.sst        = SPP::SST::DATA;
			state.seq        = 0;
			state.ack        = 0;
			state.alloc      = 4;
			// save for later use
			stateMap[key] = state;
		}

		if (spp.control.isSystem()) {
			// system packet
			if (spp.control.isSendAck()) {
				SPP reply;
				reply.control = SPP::Control::BIT_SYSTEM;
				reply.sst = SPP::SST::DATA;
				reply.idSrc = state.idLocal;
				reply.idDst = spp.idSrc;
				reply.seq   = state.seq;
				reply.ack   = state.ack;
				reply.alloc = state.alloc;

				DefaultListener::transmit(data, reply);
				return;
			}

		} else {
			// data packet
			// collect data and send data to client
		}

	}

	logger.info("CourierListener::run STOP");
}

