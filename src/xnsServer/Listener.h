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
// Listeners.h
//

#pragma once

#include "../xns/XNS.h"
#include "../xns/Config.h"
#include "../xns/RIP.h"
#include "../xns/Echo.h"
#include "../xns/Error.h"
#include "../xns/PEX.h"
#include "../xns/SPP.h"
#include "../xns/Boot.h"

#include "../courierImpl/Service.h"

namespace XNS::Server {
	using Network::Driver;
	using Courier::Services;
	using Courier::BLOCK;

	// forward declaration
	class Server;

	class Listener {
	public:
		enum class State {
			NEW, INITIALIZED, STARTED, STOPPED,
		};
		static const char* toString(State value);

		void initListener  (Server* server);
		void startListener ();
		void stopListener  ();

		virtual void handle(const Data& data) = 0;

		virtual ~Listener() {}
		Listener(const Listener& that) {
			this->myName       = that.myName;
			this->mySocket     = that.mySocket;
			this->myAutoDelete = that.myAutoDelete;
			this->myState      = that.myState;
		}
		Listener& operator = (const Listener& that) {
			this->myName       = that.myName;
			this->mySocket     = that.mySocket;
			this->myAutoDelete = that.myAutoDelete;
			this->myState      = that.myState;
			return *this;
		}

		Listener() : myName(nullptr), mySocket(0), myAutoDelete(false), myState(State::NEW) {}

		Listener(const char* name_, uint16_t socket_) : myName(name_), mySocket(socket_), myAutoDelete(false), myState(State::NEW) {}

		std::string toString();

		const char* name  () const {
			return myName;
		}
		uint16_t     socket() const {
			return mySocket;
		}
		bool        autoDelete() const {
			return myAutoDelete;
		}
		State       state() const {
			return myState;
		}
		void name(const char* newValue) {
			myName = newValue;
		}
		void socket(uint16_t newValue) {
			mySocket = newValue;
		}
		void autoDelete(bool newValue) {
			myAutoDelete = newValue;
		}

		static void transmit(const Data& data, const RIP&   rip);
		static void transmit(const Data& data, const Echo&  echo);
		static void transmit(const Data& data, const Error& error);
		static void transmit(const Data& data, const PEX&   pex);
		static void transmit(const Data& data, const SPP&   spp);
		static void transmit(const Data& data, const Boot&  boot);

	protected:
		const char* myName;
		uint16_t     mySocket;
		bool        myAutoDelete;
		State       myState;

		virtual void init  (Server* server) = 0;
		virtual void start ()               = 0;
		virtual void stop  ()               = 0;

		// Transmit idp data with padding and set checksum
		static void transmit(Driver* driver, uint64_t dst, uint64_t src, const IDP& idp);

		// for RIP broadcast
		static void transmit(const Context* context, uint64_t dst, const IDP& idp) {
			transmit(context->driver, dst, context->address, idp);
		}

		// transmit idp packet
		static void transmit(const Data& data, const IDP& idp) {
			transmit(data.context, data.ethernet.src, idp);
		}

		// initialize idp for transmit
		static void setIDP(const Data& data, uint8_t type, BLOCK& block, IDP& idp);
	};


	class Listeners {
	public:
		Listeners() : server(nullptr), started(false) {}

		// life cycle management
		void init(Server* server_) {
			server = server_;
		}

		void start();
		void stop();

		// Add listener to map
		// call listener.init()
		// If listeners is running, call listener.start()
		void add(uint16_t socket, Listener* listener);
		void add(Listener* listener) {
			add(listener->socket(), listener);
		}
		// Remove listener from map
		// If listeners is running, call listener.stop().
		// If listener is autoDelete, delete listener.
		void remove(uint16_t socket);
		Listener* getListener(uint16_t socket);

		// Well-known socket numbers range from 1 to 3000 decimal
		uint16_t getUnusedSocket() const;

	protected:
		Server* server;
		bool    started;

		// FIXME delete inactive socket - session timeout
		std::map<uint16_t, Listener*> map;
		//   socket
		mutable std::mutex                  mapMutex;
	};

}
