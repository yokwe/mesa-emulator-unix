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
// Server.h
//

#pragma once

#include "../util/Network.h"

#include "../xns/XNS.h"
#include "../xns/Config.h"
#include "../xns/RIP.h"
#include "../xns/Echo.h"
#include "../xns/Error.h"
#include "../xns/PEX.h"
#include "../xns/SPP.h"
#include "../xns/Boot.h"

#include "../courier/Service.h"

#include "Listener.h"

#include <QtConcurrent/QtConcurrent>

namespace XNS::Server2 {
	using Network::Driver;
	using Network::Packet;
	using Courier::Service;
	using Courier::Services;
	using Courier::ProgramVersion;

	class Server {
		Config        config;
		Context       context;

		Listeners     listeners;
		Services      services;

		bool          stopFuture;
		QFuture<void> future;

	public:
		void add(Listener listener) {
			listeners.add(listener);
		}
		void add(Service service) {
			services.add(service);
		}

		// life cycle management
		void init(const QString& path);
		void start();
		void stop();

		bool isRnning() {
			return future.isRunning();
		}

		// for future
		void run();
	};

}


