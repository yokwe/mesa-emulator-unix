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


namespace XNS::Server2 {
	using XNS::IDP;
	using XNS::RIP;
	using XNS::Echo;
	using XNS::PEX;
	using XNS::SPP;
	using XNS::Boot;


	class Listener {
	public:
		class Base {
		public:
			virtual ~Base() {}
			virtual quint16     socket()                  = 0;
			virtual const char* name  ()                  = 0;
			virtual void        init  (Config*, Context*) = 0;
			virtual void        start ()                  = 0;
			virtual void        stop  ()                  = 0;

			virtual void        handle(const Data& data) = 0;
		};

		std::function<quint16    (void)>              socket;
		std::function<const char*(void)>              name;
		std::function<void       (Config*, Context*)> init;
		std::function<void       (void)>              start;
		std::function<void       (void)>              stop;
		std::function<void       (const Data&)>       handle;

		Listener() :
			socket(nullptr), name(nullptr), init(nullptr), start(nullptr), stop(nullptr), handle(nullptr) {}
		Listener(Base& base) :
			socket ([&base]()                                {return base.socket();}),
			name   ([&base]()                                {return base.name();}),
			init   ([&base](Config* config, Context* context){base.init(config, context);}),
			start  ([&base]()                                {base.start();}),
			stop   ([&base]()                                {base.stop();}),
			handle ([&base](const Data& data)                {base.handle(data);}) {}

		bool isNull() {
			return socket == nullptr;
		}
	};


	class Listeners {
	public:
		void add(Listener listener);
		Listener getListener(quint16 socket);

		// life cycle management
		void init(Config* config, Context* context);
		void start();
		void stop();

	protected:
		QMap<quint16, Listener> map;
		//   socket
	};


	class DefaultListener : public Listener::Base {
	public:
		DefaultListener() : config(nullptr), context(nullptr) {}
		virtual ~DefaultListener() {}

		void init(Config* config_, Context* context_) {
			config  = config_;
			context = context_;
		}

		void start() {}
		void stop () {}

	protected:
		Config*  config;
		Context* context;

		// for RIP broadcast
		static void transmit(const Context& context, quint64 dst, const IDP& idp);

		// transmit idp packet
		static void transmit(const Data& data, const IDP& idp) {
			transmit(data.context, data.ethernet.src, idp);
		}

		static void transmit(const Data& data, const RIP&   rip);
		static void transmit(const Data& data, const Echo&  echo);
		static void transmit(const Data& data, const Error& error);
		static void transmit(const Data& data, const PEX&   pex);
		static void transmit(const Data& data, const SPP&   spp);
		static void transmit(const Data& data, const Boot&  boot);

	private:
		// initialize idp for transmit
		static void init(const Data& data, quint8 type, BLOCK& block, IDP& idp);

	};

}


/*
	class DefaultRIP : public Default {
	public:
		quint16 socket(){
			return XNS::IDP::Socket::RIP;
		}
		QString name() {
			return "RIPListener";
		}
		void start();
		void stop();

		void handle(const Data& data);

	private:
		// process received data
		void receive(const Data& data, const RIP&   rip);
	};

	class DefaultEcho : public Default {
	public:
		quint16 socket(){
			return XNS::IDP::Socket::ECHO;
		}
		QString name() {
			return "RIPListener";
		}
		void start();
		void stop();

		void handle(const Data& data);

	private:
		// process received data
		void receive(const Data& data, const Echo&  echo);
	};


 */
