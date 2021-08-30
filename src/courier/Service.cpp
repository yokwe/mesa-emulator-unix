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
// Service.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("service");

#include "Service.h"


using XNS::Data;
using XNS::PEX;

//
// Courier::Service
//
void Courier::Service::add(Procedure* procedure) {
	quint16 key = procedure->procedure();
	if (map.contains(key)) {
		logger.error("Unexpected");
		logger.error("  service   %u-%u %s", program(), version(), name());
		logger.error("  procedure %u %s", procedure->procedure(), procedure->name());
		ERROR();
	} else {
		map[key] = procedure;
	}
}
QString Courier::Service::toString() const {
	return QString::asprintf("%d-%d %s", myProgram, myVersion, myName);
}

Courier::Procedure* Courier::Service::getProcedure(const quint16 procedure) const {
	if (map.contains(procedure)) {
		return map[procedure];
	} else {
		return nullptr;
	}
}
void Courier::Service::call(const Data& data, const PEX& pex, const Protocol3Body::CallBody& callBody) const {
	Procedure* procedure = getProcedure(callBody.procedure);
	if (procedure == nullptr) {
		logger.error("callBody %s", callBody.toString());
		ERROR();
	} else {
		procedure->call(data, pex, callBody);
	}
}


//
// Courier::Services
//
void Courier::Services::init() {
	// call init of service in map
	logger.debug("Services::init");
	for(auto i = map.begin(); i != map.end(); i++) {
		Service* service = i.value();
		logger.info("Services::init  %s", service->toString());
		service->init();
	}
}
void Courier::Services::start() {
	// call init of service in map
	logger.debug("Services::start");
	for(auto i = map.begin(); i != map.end(); i++) {
		Service* service = i.value();
		logger.info("Services::start %s", service->toString());
		service->start();
	}
}
void Courier::Services::stop() {
	// call init of service in map
	logger.debug("Services::stop");
	for(auto i = map.begin(); i != map.end(); i++) {
		Service* service = i.value();
		logger.info("Services::stop  %s", service->toString());
		service->stop();
	}
}
void Courier::Services::add(Service* service) {
	ProgramVersion programVersion(service->program(), service->version());

	if (map.contains(programVersion)) {
		logger.error("Unexpected");
		logger.error("  service  %d-%d %s", service->program(), service->version(), service->name());
		ERROR();
	} else {
		map[programVersion] = service;
	}
}
Courier::Service* Courier::Services::getService(const ProgramVersion& programVersion) const {
	if (map.contains(programVersion)) {
		return map[programVersion];
	} else {
		return nullptr;
	}
}
void Courier::Services::call(const Data& data, const PEX& pex, const Protocol3Body& body) const {
	Protocol3Body::CallBody callBody;
	body.get(callBody);

	ProgramVersion programVersion((quint32)callBody.program, (quint16)callBody.version);
	Service* service = getService(programVersion);
	if (service == nullptr) {
		logger.error("Unexpected");
		logger.error("  callBody %s", callBody.toString());
		ERROR();
	} else {
		Procedure* procedure = service->getProcedure(callBody.procedure);
		if (procedure == nullptr) {
			logger.error("Unexpected");
			logger.error("  callBody %s", callBody.toString());
			ERROR();
		} else {
			procedure->call(data, pex, callBody);
		}
	}
}


//
// Courier::Procedure
//
QString Courier::Procedure::toString() const {
	return QString::asprintf("%d %s", myProcedure, myName);
}
