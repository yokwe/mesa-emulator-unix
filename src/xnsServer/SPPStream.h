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
// SPPStream.h
//

#pragma once

#include "../xnsServer/SPPListener.h"


class SPPStream : public SPPListener {
public:
	SPPStream(const char* name, quint16 socket) : SPPListener(name, socket) {}

	virtual ~SPPStream() {}

	void start();
	void stop();

	void handle(const XNS::Data& data, const XNS::SPP& spp);

protected:
	class State {
	public:
		quint64  time;

		quint64  remoteHost;
		quint16  remoteSocket;
		quint16  remoteID;

		quint16  localSocket;
		quint16  localID;

		quint16  sst;

		quint16  seq;     // sequence
		quint16  ack;     // acknowledgment
		quint16  alloc;   // allocation

		State() : time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0), sst(0), seq(0), ack(0), alloc(0) {}
		State(const State& that) {
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

	class Key {
	public:
		quint64 host;
		quint16 socket;
		quint16 id;

		Key() : host(0), socket(0), id(0) {}
		Key(const Key& that) {
			this->host   = that.host;
			this->socket = that.socket;
			this->id     = that.id;
		}
		Key& operator = (const Key& that) {
			this->host   = that.host;
			this->socket = that.socket;
			this->id     = that.id;
			return *this;
		}

		Key(quint64 host_, quint16 socket_, quint16 id_) : host(host_), socket(socket_), id(id_) {}

		QString toString() {
			return QString("%1-%2-%3").arg(XNS::Host::toHexaDecimalString(host)).arg(XNS::Socket::toString(socket)).arg(id, 4, 16, QChar('0'));
		}

		bool operator == (const Key& that) const {
			return this->host == that.host && this->socket == that.socket && this->id == that.id;
		}
		bool operator < (const Key& that) const {
			if (this->host == that.host) {
				if (this->socket == that.socket) {
					return this->id < that.id;
				} else {
					return this->socket < that.socket;
				}
			} else {
				return this->host < that.host;
			}
		}
	};

	static QMap<Key, State> stateMap;
	static QList<SPPStream> clientList;


};

