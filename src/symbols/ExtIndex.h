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
// ExtIndex.h
//

#ifndef EXTINDEX_H__
#define EXTINDEX_H__

#include "../util/Util.h"
#include "../mesa/MesaBasic.h"

#include "BCD.h"
#include "Symbols.h"

class TreeLink;


//ExtIndex: TYPE = Base RELATIVE POINTER [0..Limit) TO ExtRecord;
//ExtNull: ExtIndex = LAST[ExtIndex];
class ExtIndex {
private:
	static constexpr const char* PREFIX = "ext";
	static const CARD16  EXT_NULL = Symbols::LIMIT - 1;

	Symbols*   symbols;
	CARD16     index;

public:
	static void checkAll();
	static ExtIndex* getNull();
	static ExtIndex* getInstance(Symbols* symbols, CARD16 index);

	bool isNull() const {
		return index == EXT_NULL;
	}
	QString toString() const;
	const ExtRecord& getValue() const;

private:
	typedef Symbols::Key Key;
	static QMap<Key, ExtIndex*> all;

	ExtIndex(Symbols* symbols, CARD16 index_);
};


//ExtRecord: PUBLIC TYPE = RECORD [
//  type (0:0..1): Symbols.ExtensionType[value..default],
//  sei (0:2..15): Symbols.ISEIndex,
//  tree (1:0..15): Tree.Link]

class ExtRecord {
private:
	const Symbols*  symbols;
	const CARD16    index;

public:
	static ExtRecord* getInstance(Symbols* symbols, CARD16 index);
	static ExtRecord* find(Symbols* symbols, CARD16 index);
	static ExtRecord* find(const SEIndex* sei);


	typedef Symbols::ExtensionType ExtensionType;
	static QString toString(ExtensionType value) {
		return Symbols::toString(value);
	}

	const ExtensionType type; // value..default
	const SEIndex*      sei;
	const TreeLink*     tree;

	QString toString() const;

private:
	typedef Symbols::Key Key;
	static QMap<Key, ExtRecord*> all;

	ExtRecord(Symbols* symbols, CARD16 index, ExtensionType type, SEIndex* sei, TreeLink* tree);
};

#endif
