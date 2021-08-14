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
// XNS.cpp
//

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
QString XNS::IDP::Host::toOctalString(quint64 value) {
	return QString::asprintf("%llob", value);
}
QString XNS::IDP::Host::toDecimalString(quint64 value) {
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
QString XNS::IDP::Host::toHexaDecimalString(quint64 value, QString sep) {
	QStringList list;
	list += QString::asprintf("%02X", (int)(value >> 40) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 32) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 24) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >> 16) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >>  8) & 0xFF);
	list += QString::asprintf("%02X", (int)(value >>  0) & 0xFF);
	return list.join(sep);
}


static quint64 getValue(int base, int factor, QStringList list) {
	quint64 ret = 0;
	for(int i = 1; i < list.size(); i++) {
		ret *= factor;
		ret += list[i].toInt(0, base);
	}
	return ret;
}

// supposed separator used in toHexaDecimalString() format
#define HEXSEP "[-:]?"

quint64 XNS::IDP::Host::fromString(QString string) {
	static QRegExp dec4("([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})");
	static QRegExp dec5("([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})");
	static QRegExp hex("([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])");
	static QRegExp oct("([0-7]+)b");

	if (dec4.exactMatch(string)) {
		QStringList list = dec4.capturedTexts();
		return getValue(10, 1000, list);
	}
	if (dec5.exactMatch(string)) {
		QStringList list = dec5.capturedTexts();
		logger.info("list %d", list.size());
		return getValue(10, 1000, list);
	}
	if (hex.exactMatch(string)) {
		QStringList list = hex.capturedTexts();
		return getValue(16, 256, list);
	}
	if (oct.exactMatch(string)) {
		QStringList list = oct.capturedTexts();
		return list[1].toLongLong(0, 8);
	}
	logger.error("Unexpected");
	logger.error("  string = %s!", string);
	ERROR();
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
	FROM_BYTE_BUFFER(bb, checksum);
	FROM_BYTE_BUFFER(bb, length);
	FROM_BYTE_BUFFER(bb, control);
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, dstNet);
	FROM_BYTE_BUFFER(bb, dstHost);
	FROM_BYTE_BUFFER(bb, dstSocket);
	FROM_BYTE_BUFFER(bb, srcNet);
	FROM_BYTE_BUFFER(bb, srcHost);
	FROM_BYTE_BUFFER(bb, srcSocket);
}
void XNS::IDP::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, checksum);
	TO_BYTE_BUFFER(bb, length);
	TO_BYTE_BUFFER(bb, control);
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, dstNet);
	TO_BYTE_BUFFER(bb, dstHost);
	TO_BYTE_BUFFER(bb, dstSocket);
	TO_BYTE_BUFFER(bb, srcNet);
	TO_BYTE_BUFFER(bb, srcHost);
	TO_BYTE_BUFFER(bb, srcSocket);
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
	FROM_BYTE_BUFFER(bb, dst);
	FROM_BYTE_BUFFER(bb, src);
	FROM_BYTE_BUFFER(bb, type);
}
void XNS::Ethernet::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, dst);
	TO_BYTE_BUFFER(bb, src);
	TO_BYTE_BUFFER(bb, type);
}

