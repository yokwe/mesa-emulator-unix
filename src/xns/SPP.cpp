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
// SPP.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-spp");

#include "../util/JSONUtil.h"

#include "SPP.h"


//
// XNS::SPP::SST
//
NameMap::Map<quint16> XNS::SPP::SST::nameMap(NameMap::toString16u, {
	{DATA,         "DATA"},
	{BULK,         "BULK"},
	{CLOSE,        "CLOSE"},
	{CLOSE_REPLY,  "CLOSE_REPLY"},
});


//
// XNS::SPP::Control
//
QString XNS::SPP::Control::toString() const {
	QStringList list;
	if (isSystem())         list += "SYS";
	if (isSendAck())        list += "SEND_ACK";
	if (isAttention())      list += "ATT";
	if (isEndOfMessage())   list += "EOM";
	if (value() & BIT_UNUSED) list += QString::asprintf("UNUSED_%1X", value() & BIT_UNUSED);

	return QString("{%1}").arg(list.join(" "));
}


//
// XNS::SPP
//
QString XNS::SPP::toString() const {
	return QString("%1 %2  %3 %4  %5-%6-%7").
		arg(control.toString()).
		arg(sst.toString()).
		arg(QString("%1").arg((quint16)idSrc, 4, 16, QChar('0')).toUpper()).
		arg(QString("%1").arg((quint16)idDst, 4, 16, QChar('0')).toUpper()).
		arg((quint16)seq).arg((quint16)ack).arg((quint16)alloc);
}
void XNS::SPP::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, control);
	FROM_BYTE_BUFFER(bb, sst);
	FROM_BYTE_BUFFER(bb, idSrc);
	FROM_BYTE_BUFFER(bb, idDst);
	FROM_BYTE_BUFFER(bb, seq);
	FROM_BYTE_BUFFER(bb, ack);
	FROM_BYTE_BUFFER(bb, alloc);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::SPP::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, control);
	TO_BYTE_BUFFER(bb, sst);
	TO_BYTE_BUFFER(bb, idSrc);
	TO_BYTE_BUFFER(bb, idDst);
	TO_BYTE_BUFFER(bb, seq);
	TO_BYTE_BUFFER(bb, ack);
	TO_BYTE_BUFFER(bb, alloc);
	TO_BYTE_BUFFER(bb, block);
}
