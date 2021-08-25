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
// Config.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-config");

#include "XNS.h"

#include "Config.h"


// XNS::Config::Network::Entry
void XNS::Config::Network::Entry::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(net);
	GET_JSON_OBJECT(hop);
}
QJsonObject XNS::Config::Network::Entry::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(net);
	SET_JSON_OBJECT(hop);
	return jsonObject;
}

// XNS::Config::Network
void XNS::Config::Network::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(interface);
	GET_JSON_OBJECT(local);
	GET_JSON_OBJECT(list);
}
QJsonObject XNS::Config::Network::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(interface);
	SET_JSON_OBJECT(list);
	return jsonObject;
}

// XNS::Config::Host::Entry
void XNS::Config::Host::Entry::fromJsonObject(const QJsonObject& jsonObject) {
	QString valueString;
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT2(value, valueString);
	value = XNS::Host::fromString(valueString);
}
QJsonObject XNS::Config::Host::Entry::toJsonObject() const {
	QString valueString = XNS::Host::toHexaDecimalString(value, ":");
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT2(value, valueString);
	return jsonObject;
}
// XNS::Config::Host
void XNS::Config::Host::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(list);
}
QJsonObject XNS::Config::Host::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(list);
	return jsonObject;
}

// XNS::Config::Time
void XNS::Config::Time::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(offsetDirection);
	GET_JSON_OBJECT(offsetHours);
	GET_JSON_OBJECT(offsetMinutes);
	GET_JSON_OBJECT(dstStart);
	GET_JSON_OBJECT(dstEnd);
}
QJsonObject XNS::Config::Time::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(offsetDirection);
	SET_JSON_OBJECT(offsetHours);
	SET_JSON_OBJECT(offsetMinutes);
	SET_JSON_OBJECT(dstStart);
	SET_JSON_OBJECT(dstEnd);
	return jsonObject;
}

// XNS::Config
void XNS::Config::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(network);
	GET_JSON_OBJECT(host);
	GET_JSON_OBJECT(time);
}
QJsonObject XNS::Config::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(network);
	SET_JSON_OBJECT(host);
	SET_JSON_OBJECT(time);
	return jsonObject;
}
