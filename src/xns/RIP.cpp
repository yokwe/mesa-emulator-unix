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
// RIP.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-rip");

#include "../util/JSONUtil.h"

#include "RIP.h"


//
// XNS::RIP::Type
//
NameMap::Map<quint16> XNS::RIP::Type::nameMap(NameMap::toString16u, {{REQUEST, "REQUEST"}, {RESPONSE, "RESPONSE"}});


//
// XNS::RIP::Entry
//
QString XNS::RIP::Entry::toString() const {
	return QString("{%1 %2}").arg(net.toString()).arg((quint16)hop);
}
void XNS::RIP::Entry::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, net);
	FROM_BYTE_BUFFER(bb, hop);
}
void XNS::RIP::Entry::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, net);
	TO_BYTE_BUFFER(bb, hop);
}


//
// XNS::RIP
//
QString XNS::RIP::toString() const {
	QStringList list;
	for(auto e: entryList) {
		list += e.toString();
	}
	return QString("%1 %2").arg(type.toString()).arg(list.join(" "));
}
void XNS::RIP::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);

	for(;;) {
		if (bb.remaining() == 0) break;

		Entry entry;
		FROM_BYTE_BUFFER(bb, entry);
		entryList += entry;
	}
}
void XNS::RIP::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	for(auto e: entryList) {
		TO_BYTE_BUFFER(bb, e);
	}
}
