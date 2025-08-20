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
// HTIndex.h
//

#ifndef HTINDEX_H__
#define HTINDEX_H__

#include "../util/Util.h"
#include "../mesa/MesaBasic.h"

#include "BCD.h"
#include "Symbols.h"


//HTIndex: TYPE = CARDINAL [0..Limit/2);
//HTNull: HTIndex = FIRST[HTIndex];
class HTIndex {
private:
	static constexpr const char* PREFIX = "ht";
	static const CARD16  HT_NULL = 0;

	Symbols*  symbols;
	CARD16    index;

public:
	static void checkAll();
	static HTIndex* getNull();
	static HTIndex* getInstance(Symbols* symbols_, CARD16 index_);

	bool isNull() const {
		return index == HT_NULL;
	}
	std::string toString() const;

	const HTRecord& getValue() const;
private:
	typedef Symbols::Key Key;
	static QMap<Key, HTIndex*> all;

	HTIndex(Symbols* symbols, CARD16 index);
};


//HTRecord: TYPE = RECORD [
//  anyInternal, anyPublic: BOOLEAN,
//  link: HTIndex,
//  ssIndex: CARDINAL];
class HTRecord {
private:
	const Symbols*  symbols;
	const CARD16    index;

public:
	static HTRecord* getInstance(Symbols* symbols, CARD16 index, CARD16 lastSSIndex);
	static HTRecord* find(Symbols* symbols, CARD16 index);

	const bool    anyInternal;
	const bool    anyPublic;
	const CARD16  link;
	const CARD16  ssIndex;
	const std::string value;

	std::string toString() const;

	const Symbols* getSymbols() const {
		return symbols;
	}
	CARD16 getIndex() const {
		return index;
	}

private:
	typedef Symbols::Key Key;
	static QMap<Key, HTRecord*> all;

	HTRecord(Symbols* symbols, CARD16 index, bool anyInternal, bool anyPublic, CARD16 link, CARD16 ssIndex, std::string value);
};

#endif
