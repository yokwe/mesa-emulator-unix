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

#include "../util/JSONUtil.h"

#include "XNS.h"


//
// XNS
//

static XNS::Config config;

XNS::Config XNS::loadConfig(QString path) {
	QJsonObject jsonObject = JSONUtil::loadObject(path);
	config.fromJsonObject(jsonObject);

	// add name to
	for(auto e: config.netList) {
		XNS::IDP::Net::addNameMap(e.value, e.name);
	}
	for(auto e: config.hostList) {
		XNS::IDP::Host::addNameMap(e.value, e.name);
	}

	logger.info("config localNet = %d", config.localNet);
	for(auto e: config.netList) {
		logger.info("config net  %4d  %s", e.value, e.name);
	}
	for(auto e: config.hostList) {
		logger.info("config host %s  %llX  %s", XNS::IDP::Host::toHexaDecimalString(e.value, ":"), e.value, e.name);
	}


	return config;
}


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
	ret[ERROR_]  = "ERROR";
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
		return toHexaDecimalString(":");
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
	ret[ERROR_]    = "ERROR";
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
	int pos = bb.position();

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

	bb.limit(pos + length);
	FROM_BYTE_BUFFER(bb, block);

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
	TO_BYTE_BUFFER(bb, block);
}

quint16 XNS::IDP::getChecksum(const ByteBuffer::Buffer& bb) {
	quint16 ret;
	bb.read16(bb.position() + OFFSET_CHECKSUM, ret);
	return ret;
}
void XNS::IDP::setChecksum(ByteBuffer::Buffer& bb, quint16 newValue) {
	bb.write16(bb.position() + OFFSET_CHECKSUM, newValue);
}
quint16 XNS::IDP::computeChecksum(const ByteBuffer::Buffer& bb) {
	quint8* data   = bb.data();
	int     offset = bb.position();

	// read length field of IDP packet
	quint16 length;
	bb.read16(offset + OFFSET_LENGTH, length);

	// move offset to length field
	offset += OFFSET_LENGTH; // increment offset
	length -= OFFSET_LENGTH; // decrement length

    quint32 s = 0;
    for(int i = 0; i < length; i += 2) {
 		quint32 w = (data[offset + 0] << 8) | data[offset + 1];
 		offset += 2;

		// add w to s
		s += w;
		// if there is overflow, increment t
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
		// shift left
		s <<= 1;
		// if there is overflow, increment t
		if (0x10000U <= s) s = (s + 1) & 0xFFFFU;
    }
    return (quint16)s;
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
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Ethernet::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, dst);
	TO_BYTE_BUFFER(bb, src);
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Routing::Type
//
QString XNS::Routing::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::Routing::Type::initNameMap() {
	QMap<quint16, QString> ret;
	nameMap[REQUEST]  = "REQUEST";
	nameMap[RESPONSE] = "RESPONSE";
	return ret;
}
QMap<quint16, QString> XNS::Routing::Type::nameMap = initNameMap();


//
// XNS::Routing::Entry
//
QString XNS::Routing::Entry::toString() const {
	return QString("{%1 %2}").arg(net.toString()).arg((quint16)hop);
}
void XNS::Routing::Entry::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, net);
	FROM_BYTE_BUFFER(bb, hop);
}
void XNS::Routing::Entry::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, net);
	TO_BYTE_BUFFER(bb, hop);
}


//
// XNS::Routing
//
QString XNS::Routing::toString() const {
	QStringList list;
	for(auto e: entryList) {
		list += e.toString();
	}
	return QString("%1 %2").arg(type.toString()).arg(list.join(" "));
}
void XNS::Routing::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);

	for(;;) {
		if (bb.remaining() == 0) break;;

		Entry entry;
		FROM_BYTE_BUFFER(bb, entry);
		entryList += entry;
	}
}
void XNS::Routing::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	for(auto e: entryList) {
		TO_BYTE_BUFFER(bb, e);
	}
}


//
// XNS::PEX::Type
//
QString XNS::PEX::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::PEX::Type::initNameMap() {
	QMap<quint16, QString> ret;
	nameMap[UNSPEC]    = "UNSPEC";
	nameMap[TIME]      = "TIME";
	nameMap[CHS]       = "CHS";
	nameMap[TELEDEBUG] = "TELEDEBUG";
	return ret;
}
QMap<quint16, QString> XNS::PEX::Type::nameMap = initNameMap();


//
// XNS::PEX
//
QString XNS::PEX::toString() const {
	return QString("%1 %2").arg(QString("%1").arg((quint32)id, 4, 16, QChar('0')).toUpper()).arg(type.toString());
}
void XNS::PEX::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, id);
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::PEX::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, id);
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Error::Type
//
QString XNS::Error::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::Error::Type::initNameMap() {
	QMap<quint16, QString> ret;
	nameMap[UNSPEC]               = "UNSPEC";
	nameMap[BAD_CHECKSUM]         = "BAD_CHECKSUM";
	nameMap[NO_SOCKET]            = "NO_SOCKET";
	nameMap[RESOURCE_LIMIT]       = "RESOURCE_LIMIT";
	nameMap[LISTEN_REJECT]        = "LISTEN_REJECT";
	nameMap[INVALID_PACKET_TYPE]  = "INVALID_PACKET_TYPE";
	nameMap[PROTOCOL_VIOLATION]   = "PROTOCOL_VIOLATION";

	nameMap[UNSPECIFIED_IN_ROUTE] = "UNSPECIFIED_IN_ROUTE";
	nameMap[INCONSISTENT]         = "INCONSISTENT";
	nameMap[CANT_GET_THERE]       = "CANT_GET_THERE";
	nameMap[EXCESS_HOPS]          = "EXCESS_HOPS";
	nameMap[TOO_BIG]              = "TOO_BIG";
	nameMap[CONGESTION_WARNING]   = "CONGESTION_WARNING";
	nameMap[CONGESTION_DISCARD]   = "CONGESTION_DISCARD";
	return ret;
}
QMap<quint16, QString> XNS::Error::Type::nameMap = initNameMap();


//
// XNS::Error
//
QString XNS::Error::toString() const {
	return QString("%1 %2 %3").arg(type.toString()).arg(QString("%1").arg((quint16)param, 4, 16, QChar('0')).toUpper()).arg(block.toString());
}
void XNS::Error::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, param);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Error::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, param);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Echo::Type
//
QString XNS::Echo::Type::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%04X", value);
	}
}
QMap<quint16, QString> XNS::Echo::Type::initNameMap() {
	QMap<quint16, QString> ret;
	nameMap[REQUEST] = "REQUEST";
	nameMap[REPLY]   = "REPLY";
	return ret;
}
QMap<quint16, QString> XNS::Echo::Type::nameMap = initNameMap();


//
// XNS::Echo
//
QString XNS::Echo::toString() const {
	return QString("%1 %2").arg(type.toString()).arg(block.toString());
}
void XNS::Echo::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Echo::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::SPP::SST
//
QString XNS::SPP::SST::toString() const {
	if (nameMap.contains(value)) {
		return nameMap[value];
	} else {
		return QString::asprintf("%02X", value);
	}
}
QMap<quint8, QString> XNS::SPP::SST::initNameMap() {
	QMap<quint8, QString> ret;
	nameMap[DATA]        = "DATA";
	nameMap[BULK]        = "BULK";
	nameMap[CLOSE]       = "CLOSE";
	nameMap[CLOSE_REPLY] = "CLOSE_REPLY";
	return ret;
}
QMap<quint8, QString> XNS::SPP::SST::nameMap = initNameMap();


//
// XNS::SPP::Control
//
QString XNS::SPP::Control::toString() const {
	QStringList list;
	if (isSystem())         list += "SYS";
	if (isSendAck())        list += "SEND_ACK";
	if (isAttention())      list += "ATT";
	if (isEndOfMessage())   list += "EOM";
	if (value & BIT_UNUSED) list += QString::asprintf("UNUSED_%1X", value & BIT_UNUSED);

	return QString("{%1}").arg(list.join(" "));
}


//
// XNS::SPP
//
QString XNS::SPP::toString() const {
	return QString("%1 %2  %3 %4  %5-%6-%7").
		arg(control.toString()).arg(sst.toString()).
		arg((quint16)idSrc, 4, QChar('0')).arg((quint16)idDst, 4, QChar('0')).
		arg((quint16)seq).arg((quint16)ack).arg((quint16)alloc);
}
void XNS::SPP::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, control);
	FROM_BYTE_BUFFER(bb, sst);
	FROM_BYTE_BUFFER(bb, idSrc);
	FROM_BYTE_BUFFER(bb, idDst);
	FROM_BYTE_BUFFER(bb, seq);
	FROM_BYTE_BUFFER(bb, ack);
	FROM_BYTE_BUFFER(bb, alloc);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::SPP::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, control);
	TO_BYTE_BUFFER(bb, sst);
	TO_BYTE_BUFFER(bb, idSrc);
	TO_BYTE_BUFFER(bb, idDst);
	TO_BYTE_BUFFER(bb, seq);
	TO_BYTE_BUFFER(bb, ack);
	TO_BYTE_BUFFER(bb, alloc);
	TO_BYTE_BUFFER(bb, block);
}




