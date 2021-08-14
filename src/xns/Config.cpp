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
// Config.h
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-config");

#include "XNS.h"

#include "Config.h"


// XNS::Config::Net
void XNS::Config::Net::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(value);
}
QJsonObject XNS::Config::Net::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(value);
	return jsonObject;
}

//     52-242-768-273 - win98   - GVWin on Win98 (VMware ESXI virtual machine)
// 200-639-155-12-424 - freebsd - epair0b on dev-base.lan - FreeBSD jail
//      52-228-11-869 - ubuntu  - ens192  on ubuntu2004   - Ubuntu

// XNS::Config::Host
void XNS::Config::Host::fromJsonObject(const QJsonObject& jsonObject) {
	QString valueString;
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT2(value, valueString);
	value = XNS::IDP::Host::fromString(valueString);
}
QJsonObject XNS::Config::Host::toJsonObject() const {
	QString valueString = XNS::IDP::Host::toHexaDecimalString(value, ":");
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT2(value, valueString);
	return jsonObject;
}

// XNS::Config
void XNS::Config::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(localNet);
	GET_JSON_OBJECT(netList);
	GET_JSON_OBJECT(hostList);
}
QJsonObject XNS::Config::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(localNet);
	SET_JSON_OBJECT(netList);
	SET_JSON_OBJECT(hostList);
	return jsonObject;
}
