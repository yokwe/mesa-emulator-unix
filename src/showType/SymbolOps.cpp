/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
// SymbolOps.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../bcdFile/Symbol.h"
#include "../bcdFile/Type.h"
#include "../bcdFile/SEIndex.h"
#include "../bcdFile/SERecord.h"
#include "../bcdFile/CTXRecord.h"

#include "SymbolOps.h"

namespace SymbolOps {
//

static SEIndex seNull{SEIndex::SE_NULL, 0};
SEIndex nextSe(const Symbol& symbol, SEIndex sei) {
	if (sei.isNull()) return seNull;

	// sanity check
	if (sei.value().tag != SERecord::Tag::ID) ERROR()

    using ID = SERecord::ID;
    using Tag = ID::Tag;
    
	for(auto i = symbol.seTable.cbegin(); i != symbol.seTable.cend(); i++) {
		auto [key, value] = *i;
		if (key == sei.index()) {
			const auto& id = value->toID();
			// next for SEQUENTIAL
			auto next = i;
			next++;
			switch(id.tag) {
				case Tag::LINKED:
					return id.toLINKED().link;
				case Tag::SEQUENTIAL:
					return SEIndex(next->first, next->second);;
				case Tag::TERMINAL:
					return seNull;
				default:
					ERROR()
			}
		}
	}
	ERROR()
}
SEIndex firstCtxSe(const Symbol& symbol, CTXIndex ctx) {
	(void)symbol;
	return ctx.isNull() ? seNull : ctx.value().seList;
}

//UnderType: PROC [h: Handle, type: SEIndex] RETURNS [CSEIndex] = {
//  sei: SEIndex � type;
//  WHILE sei # SENull DO
//    WITH se: h.seb[sei] SELECT FROM
//      id => {IF se.idType # typeTYPE THEN ERROR; sei � SymbolOps.ToSei[se.idInfo]};
//      ENDCASE => EXIT;
//    ENDLOOP;
//  RETURN [LOOPHOLE[sei, CSEIndex]]};
SEIndex underType(const Symbol& symbol, SEIndex type) {
    SEIndex sei = type;
	while (!sei.isNull()) {
		if (sei.value().isID()) {
			auto se = sei.value().toID();
			if (!se.idType.isType()) ERROR()
			sei = toSei(symbol, se.idInfo);
		} else {
			break;
		}
	}
	return sei;
}

SEIndex typeLink(const Symbol& symbol, SEIndex sei) {
	sei = underType(symbol, sei);
	const auto& cons = sei.value().toCONS();
	if (cons.isRECORD()) {
		const auto& record = cons.toRECORD();
		if (record.tag == SERecord::CONS::RECORD::Tag::LINKED) {
			return record.toLINKED().linkType;
		}
	}
	return seNull;
}    

//XferMode: PROC [h: Handle, type: SEIndex] RETURNS [TransferMode] = {
//  sei: CSEIndex = UnderType[h, type];
//  RETURN [WITH t: h.seb[sei] SELECT FROM transfer => t.mode, ENDCASE => none]};
TransferMode xferMode(const Symbol& symbol, SEIndex type) {
	auto sei = underType(symbol, type);
	const auto& cons = sei.value().toCONS();
	return cons.tag == TypeClass::TRANSFER ? cons.toTRANSFER().mode : TransferMode::NONE;
}

SEIndex toSei(const Symbol& symbol, uint16_t index) {
	if (symbol.seTable.contains(index)) {
		return SEIndex(index, symbol.seTable.at(index));
	} else {
		ERROR();
	}
}

static BTIndex btNull{BTIndex::BT_NULL, 0};
BTIndex toBti(const Symbol& symbol, uint16_t index) {
	if (symbol.bodyTable.contains(index)) {
		return BTIndex(index, symbol.bodyTable.at(index));
	} else {
		ERROR();
	}
}


}