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


#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns");

#include "XNS.h"


//
// XNS::IDP::Checksum
//
void XNS::IDP::Checksum::addNameMap(quint16 value, QString name) {
	nameMap[value] = name;
}
QString XNS::IDP::Checksum::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::IDP::Checksum::initNameMap() {
	QMap<quint16, QString> ret;
	ret[NOCHECK]  = "NOCHECK";
	return ret;
}
QMap<quint16, QString> XNS::IDP::Checksum::nameMap  = initNameMap();


//
// XNS::IDP::Type
//
void XNS::IDP::Type::addNameMap(quint8 value, QString name) {
	nameMap[value] = name;
}
QString XNS::IDP::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%d", value);
	}
}
QMap<quint8, QString> XNS::IDP::Type::initNameMap() {
	QMap<quint8, QString> ret;
	ret[ROUTING] = "ROUTING";
	ret[ECHO]    = "ECHO";
	ret[ERROR]   = "ERROR";
	ret[PEX]     = "PEX";
	ret[SPP]     = "SPP";
	ret[BOOT]    = "BOOT";
	return ret;
}
QMap<quint8,  QString> XNS::IDP::Type::nameMap      = initNameMap();


//
// XNS::IDP::Net
//
void XNS::IDP::Net::addNameMap(quint32 value, QString name) {
	nameMap[value] = name;
}
QString XNS::IDP::Net::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint32, QString> XNS::IDP::Net::initNameMap() {
	QMap<quint32, QString> ret;
	ret[ALL]     = "ALL";
	ret[UNKNOWN] = "UNKNOWN";
	return ret;
}
QMap<quint32, QString> XNS::IDP::Net::nameMap       = initNameMap();


//
// XNS::IDP::Host
//
QString XNS::IDP::Host::toOctalString() const {
	return QString::asprintf("%llob", value);
}
QString XNS::IDP::Host::toDecimalString() const {
	QStringList list;
	auto n = value;
	for(;;) {
		if (n == 0) break;
		auto quotient  = n / 1000;
		auto remainder = (int)(n % 1000);

		list.prepend(QString::asprintf("%d", remainder));
		n = quotient;
	}

	return list.join("-");
}
QString XNS::IDP::Host::toHexaDecimalString(QString sep) const {
	QStringList list;
	list += QString::asprintf("%02X", (int)(value >> 40) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 32) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 24) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 16) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >>  8) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >>  0) & 0xFF);
	return list.join(sep);
}

void XNS::IDP::Host::addNameMap(quint64 value, QString name) {
	nameMap[value] = name;
}
QString XNS::IDP::Host::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return toHexaDecimalString("");
	}
}
QMap<quint64, QString> XNS::IDP::Host::initNameMap() {
	QMap<quint64, QString> ret;
	ret[ALL]     = "ALL";
	ret[UNKNOWN] = "UNKNOWN";
	return ret;
}
QMap<quint64, QString> XNS::IDP::Host::nameMap      = initNameMap();


//
// XNS::IDP::Socket
//
void XNS::IDP::Socket::addNameMap(quint16 value, QString name) {
	nameMap[value] = name;
}
QString XNS::IDP::Socket::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::IDP::Socket::initNameMap() {
	QMap<quint16, QString> ret;

	ret[ROUTING]  = "ROUTING";
	ret[ECHO]      = "ECHO";
	ret[ERROR]     = "ERROR";
	ret[ENVOY]     = "ENVOYE";
	ret[COURIER]   = "COURIER";
	ret[CHS_OLD]   = "CHS_OLD";
	ret[TIME]      = "TIME";
	ret[BOOT]      = "BOOT";
	ret[DIAG]      = "DIAG";

	ret[CHS]       = "CHS";
	ret[AUTH]      = "AUTH";
	ret[MAIL]      = "MAIL";
	ret[NETEXEC]   = "NETEXEC";
	ret[WSINFO]    = "WSINFO";
	ret[BINDING]   = "BINDING";

	ret[GERM]      = "GERM";
	ret[TELEDEBUG] = "TELEDEBUG";

	return ret;
}
QMap<quint16, QString> XNS::IDP::Socket::nameMap    = initNameMap();


//
// XNS::IDP
//
QString XNS::IDP::toString() const {
	return QString("%1 %2 %3 %4  %5-%6-%7  %8-%9-%10").
		arg(checksum.toString()).
		arg((quint16)length, 4).
		arg((quint8)control, 2, 16, QChar('0')).
		arg(type.toString()).
		arg(dstNet.toString()).arg(dstHost.toString()).arg(dstSocket.toString()).
		arg(srcNet.toString()).arg(srcHost.toString()).arg(srcSocket.toString());
}
void XNS::IDP::fromByteBuffer(Buffer& bb) {
	checksum.fromByteBuffer(bb);
	length.fromByteBuffer(bb);
	control.fromByteBuffer(bb);
	type.fromByteBuffer(bb);

	dstNet.fromByteBuffer(bb);
	dstHost.fromByteBuffer(bb);
	dstSocket.fromByteBuffer(bb);

	srcNet.fromByteBuffer(bb);
	srcHost.fromByteBuffer(bb);
	srcSocket.fromByteBuffer(bb);
}
void XNS::IDP::toByteBuffer  (Buffer& bb) const {
	checksum.toByteBuffer(bb);
	length.toByteBuffer(bb);
	control.toByteBuffer(bb);
	type.toByteBuffer(bb);

	dstNet.toByteBuffer(bb);
	dstHost.toByteBuffer(bb);
	dstSocket.toByteBuffer(bb);

	srcNet.toByteBuffer(bb);
	srcHost.toByteBuffer(bb);
	srcSocket.toByteBuffer(bb);
}


//
// XNS::Ethernet::Type
//
void XNS::Ethernet::Type::addNameMap(quint16 value, QString name) {
	nameMap[value] = name;
}
QString XNS::Ethernet::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::Ethernet::Type::initNameMap() {
	QMap<quint16, QString> ret;

	nameMap[XNS] = "XNS";
	nameMap[IP]  = "IP";
	return ret;
}
QMap<quint16, QString> XNS::Ethernet::Type::nameMap = initNameMap();


//
// XNS::Ethernet
//
QString XNS::Ethernet::toString() const {
	return QString("%1-%2-%3").arg(dst.toString()).arg(src.toString()).arg(type.toString());
}
void XNS::Ethernet::fromByteBuffer(Buffer& bb) {
	dst.fromByteBuffer(bb);
	src.fromByteBuffer(bb);
	type.fromByteBuffer(bb);
}
void XNS::Ethernet::toByteBuffer  (Buffer& bb) const {
	dst.toByteBuffer(bb);
	src.toByteBuffer(bb);
	type.toByteBuffer(bb);
}

