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

#include "Time.h"

#include "XNS.h"


//
// XNS
//
XNS::Config XNS::loadConfig(const QString& path) {
	XNS::Config config;

	QJsonObject jsonObject = JSONUtil::loadObject(path);
	config.fromJsonObject(jsonObject);

	// add known name to net
	for(auto e: config.network.list) {
		Net::addNameMap((quint32)e.net, e.name);
	}
	// add known name to host
	for(auto e: config.host.list) {
		Host::addNameMap(e.value, e.name);
	}

	// dump config values

	// config.network
	logger.info("config network interface    %s", config.network.interface);
	for(auto e: config.network.list) {
		logger.info("config network list         %s  %d  %d", e.name, e.net, e.hop);
	}

	// config.host
	for(auto e: config.host.list) {
		logger.info("config host list            %-10s  %s  %20s",
			TO_CSTRING(e.name),
			TO_CSTRING(Host::toHexaDecimalString(e.value, ":")),
			TO_CSTRING(Host::toDecimalString(e.value)));
	}

	// config time
	{
		XNS::Time::Direction direction;
		direction = (quint16)config.time.offsetDirection;

		logger.info("config time offsetDirection %s", direction.toString());
		logger.info("config time offsetHours     %d", config.time.offsetHours);
		logger.info("config time offsetMinutes   %d", config.time.offsetMinutes);
		logger.info("config time dstStart        %d", config.time.dstStart);
		logger.info("config time dstEnd          %d", config.time.dstEnd);
	}

	return config;
}


//
// XNS::Context
//
XNS::Context::Context(Config& config) {
	int localNet = 0;
	for(auto e: config.network.list) {
		if (e.hop == 0) {
			localNet = e.net;
		}
	}
	if (localNet == 0) {
		logger.error("Unexpected");
		for(auto e: config.network.list) {
			logger.error("  available network %d %d %s", e.hop, e.net, e.name);
		}
		ERROR()
	}

	Network::Device device = Network::getDevice(config.network.interface);
	address = device.address;
	driver = Network::getDriver(device);

	// set value in config.local
	config.local.host = address;
	config.local.net  = (quint32)localNet;

	logger.info("device     = %20s  %s", Host::toHexaDecimalString(device.address, ":"), device.name);
	logger.info("device     = %20s  %s", Host::toDecimalString(device.address), device.name);
	logger.info("local.net  = %s", Net::toString(config.local.net));
	logger.info("local.host = %s", Host::toString(config.local.host));
}


//
// XNS::Host
//
QString XNS::Host::toOctalString(quint64 value) {
	return QString::asprintf("%llob", value);
}
QString XNS::Host::toDecimalString(quint64 value) {
	QStringList list;
	auto n = value;
	for(;;) {
		if (n == 0) break;
		auto quotient  = n / 1000;
		auto remainder = (int)(n % 1000);

		list.prepend(QString::asprintf("%03d", remainder));
		n = quotient;
	}
	return list.join("-");
}
QString XNS::Host::toHexaDecimalString(quint64 value, QString sep) {
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

quint64 XNS::Host::fromString(QString string) {
	static QRegularExpression dec4("([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})");
	static QRegularExpression dec5("([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})-([1-9][0-9]{0,2})");
	static QRegularExpression hex("([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])");
	static QRegularExpression oct("([0-7]+)b");

	{
		auto m = dec4.match(string);
		if (m.hasMatch()) {
			QStringList list = m.capturedTexts();
			return getValue(10, 1000, list);
		}
	}
	{
		auto m = dec5.match(string);
		if (m.hasMatch()) {
			QStringList list = m.capturedTexts();
			logger.info("list %d", list.size());
			return getValue(10, 1000, list);
		}
	}
	{
		auto m = hex.match(string);
		if (m.hasMatch()) {
			QStringList list = m.capturedTexts();
			return getValue(10, 256, list);
		}
	}
	{
		auto m = oct.match(string);
		if (m.hasMatch()) {
			QStringList list = m.capturedTexts();
			return list[1].toLongLong(0, 8);
		}
	}

	logger.error("Unexpected");
	logger.error("  string = %s!", string);
	ERROR();
}

NameMap::Map<quint64> XNS::Host::nameMap(NameMap::toString64X, {
	{ALL,       "ALL"},
	{UNKNOWN,   "UNKNOWN"},
	{BFN_GVWIN, "BFN_GVWIN"},
});


//
// XNS::Ethernet::Type
//
NameMap::Map<quint16> XNS::Ethernet::Type::nameMap(NameMap::toString16X04, {{XNS, "XNS"}, {IP, "IP"}});


//
// XNS::Ethernet
//
QString XNS::Ethernet::toString() const {
	return QString("%1-%2-%3").arg(dst.toString()).arg(src.toString()).arg(type.toString());
}
void XNS::Ethernet::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, dst);
	FROM_BYTE_BUFFER(bb, src);
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Ethernet::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, dst);
	TO_BYTE_BUFFER(bb, src);
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Checksum
//
NameMap::Map<quint16> XNS::Checksum::nameMap(NameMap::toString16X04, {{NOCHECK, "NOCHECK"}});


//
// XNS::IDP::Type
//
NameMap::Map<quint8> XNS::IDP::Type::nameMap(NameMap::toString8u, {
	{RIP,    "RIP"},
	{ECHO,   "ECHO"},
	{ERROR_, "ERROR"},
	{PEX,    "PEX"},
	{SPP,    "SPP"},
	{BOOT,   "BOOT"}
});


//
// XNS::Net
//
NameMap::Map<quint32> XNS::Net::nameMap(NameMap::toString32u, {{ALL, "ALL"}, {UNKNOWN, "UNKNOWN"}});


//
// XNS::Socket
//
NameMap::Map<quint16> XNS::Socket::nameMap(NameMap::toString16X04, {
	{RIP,       "RIP"},
	{ECHO,      "ECHO"},
	{ERROR_,    "ERROR"},
	{ENVOY,     "ENVOYE"},
	{COURIER,   "COURIER"},
	{CHS_OLD,   "CHS_OLD"},
	{TIME,      "TIME"},
	{BOOT,      "BOOT"},
	{DIAG,      "DIAG"},

	{CHS,       "CHS"},
	{AUTH,      "AUTH"},
	{MAIL,      "MAIL"},
	{NETEXEC,   "NETEXEC"},
	{WSINFO,    "WSINFO"},
	{BINDING,   "BINDING"},

	{GERM,      "GERM"},
	{TELEDEBUG, "TELEDEBUG"},
});


//
// XNS::IDP
//
QString XNS::IDP::toString() const {
	QString dst = QString("%1-%2-%3").arg(dstNet.toString()).arg(dstHost.toString()).arg(dstSocket.toString());
	QString src = QString("%1-%2-%3").arg(srcNet.toString()).arg(srcHost.toString()).arg(srcSocket.toString());

	return QString("%1 %2 %3 %4 %5 %6").
		arg(checksum_.toString(), -7).
		arg((quint16)length, 4).
		arg((quint8)control, 2, 16, QChar('0')).
		arg(type.toString(), 4).
		arg(dst, -20).
		arg(src, -20);
}
void XNS::IDP::fromByteBuffer(ByteBuffer& bb) {
	int start = bb.position();

	FROM_BYTE_BUFFER(bb, checksum_);
	FROM_BYTE_BUFFER(bb, length);
	FROM_BYTE_BUFFER(bb, control);
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, dstNet);
	FROM_BYTE_BUFFER(bb, dstHost);
	FROM_BYTE_BUFFER(bb, dstSocket);
	FROM_BYTE_BUFFER(bb, srcNet);
	FROM_BYTE_BUFFER(bb, srcHost);
	FROM_BYTE_BUFFER(bb, srcSocket);

	// set bb limit by length
	bb.limit(start + (quint16)length);

	// read block after change limit
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::IDP::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, checksum_);
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

quint16 XNS::IDP::getLength(const ByteBuffer& bb) {
	quint16 newValue;
	bb.read16(bb.base() + OFFSET_LENGTH, newValue);
	return newValue;
}
void XNS::IDP::setLength(ByteBuffer& bb, quint16 newValue) {
	bb.write16(bb.base() + OFFSET_LENGTH, newValue);
}

quint16 XNS::IDP::getChecksum(const ByteBuffer& bb) {
	quint16 newValue;
	bb.read16(bb.base() + OFFSET_CHECKSUM, newValue);
	return newValue;
}
void XNS::IDP::setChecksum(ByteBuffer& bb, quint16 newValue) {
	bb.write16(bb.base() + OFFSET_CHECKSUM, newValue);
}

quint16 XNS::IDP::computeChecksum(const ByteBuffer& bb) {
	quint8* data   = bb.data();
	int     offset = bb.base();

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
// XNS::Data
//
void XNS::Data::copyFrom(const Data& that) {
	this->timeStamp = that.timeStamp;
	this->config    = that.config;
	this->context   = that.context;
	this->packet    = that.packet;
	this->ethernet  = that.ethernet;
	this->idp       = that.idp;
}
void XNS::Data::fixBlock() {
	// address of packet is changed. Need update block
	Courier::BLOCK newValue(this->packet);
	this->ethernet.updateBlock(newValue);
	this->idp.updateBlock(newValue);
}
XNS::Data::Data() {
	timeStamp = 0;
	config    = nullptr;
	context   = nullptr;
	fixBlock();
}
XNS::Data::~Data() {
	//
}
XNS::Data::Data(const Data& that) {
	copyFrom(that);
	fixBlock();
}
XNS::Data& XNS::Data::operator = (const Data& that) {
	copyFrom(that);
	fixBlock();
	return *this;
}
XNS::Data::Data(qint64 timeStamp_, Config* config_, Context* context_, Packet& packet_, Ethernet ethernet_, IDP idp_) :
	timeStamp(timeStamp_), config(config_), context(context_), packet(packet_), ethernet(ethernet_), idp(idp_) {
	fixBlock();
}

