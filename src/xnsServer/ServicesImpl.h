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
// ServicesImpl.h
//

#pragma once

#include "../xns/XNS.h"
#include "../xns/Server.h"

namespace XNS::ServicesImpl {
	using ByteBuffer::Buffer;
	using XNS::Echo;
	using XNS::Error;
	using XNS::PEX;
	using XNS::RIP;
	using XNS::Time;
	using XNS::Server::Data;
	using XNS::Courier::ExpeditedCourier;

	class RIPService : public XNS::Server::Services::RIPService {
	public:
		const char* name() {
			return "RIPService";
		}
		void receive(const Data& data, const RIP& rip);
		void receive(const Data& data, const Error& error);
	};


	class CHSService : public XNS::Server::Services::CHSService {
	public:
		const char* name() {
			return "CHSService";
		}
		void receive(const Data& data, const PEX& pex, const ExpeditedCourier& exp);
		void receive(const Data& data, const Error& error);
	};


	class TimeService : public XNS::Server::Services::TimeService {
	public:
		const char* name() {
			return "TimeService";
		}
		void receive(const Data& data, const PEX& pex, const Time& time);
		void receive(const Data& data, const Error& error);
	};


	class EchoService : public XNS::Server::Services::EchoService {
	public:
		const char* name() {
			return "EchoService";
		}
		void receive(const Data& data, const XNS::Echo& echo);
		void receive(const Data& data, const Error& error);
	};

}
