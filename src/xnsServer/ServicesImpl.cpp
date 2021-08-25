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
// ServicesImpl.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("servicesImpl");


#include "ServicesImpl.h"

namespace XNS::ServicesImpl {
	void RIPService::receive(const Data& data, const RIP& rip) {
		(void)data;
		logger.info("##  RIP %s", rip.toString());
	}
	void RIPService::receive(const Data& data, const Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}


	void CHSService::receive(const Data& data, const PEX& pex, const ExpeditedCourier& exp) {
		(void)data;
		logger.info("##  CHS %s", pex.toString());
		logger.info("        %s", exp.toString());
	}
	void CHSService::receive(const Data& data, const Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}


	void TimeService::receive(const Data& data, const PEX& pex, const Time& time) {
		(void)data;
		logger.info("##  TIME %s", pex.toString());
		logger.info("         %s", time.toString());
	}
	void TimeService::receive(const Data& data, const Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}


	void EchoService::receive(const Data& data, const Echo& echo) {
		logger.info("##  ECHO %s", echo.toString());

		if (echo.type == XNS::Echo::Type::REQUEST) {
			XNS::Echo reply;

			reply.type = XNS::Echo::Type::REPLY;
			reply.block = echo.block;

			transmit(data, reply);
		} else {
			logger.error("Unexpected");
			logger.error("  echo %s", echo.toString());
			ERROR();
		}
	}
	void EchoService::receive(const Data& data, const Error& error) {
		(void)data;
		logger.info("    ERROR %s", error.toString());
	}

}
