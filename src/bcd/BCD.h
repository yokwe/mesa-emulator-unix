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
// BCD.h
//

#pragma once

#include <compare>
#include <cstdint>
#include <string>
#include <vector>

#include "../util/ByteBuffer.h"
#include "../util/Util.h"

#include "Index.h"

constexpr const uint16_t T_LIMIT = 0177777;

// TimeStamp: TYPE = RECORD [net, host: [0..377B], time: LONG CARDINAL];
// Null: TimeStamp = TimeStamp[net: 0, host: 0, time: 0];
class Timestamp : public ByteBuffer::Readable, public HasToString {
    uint8_t  net;
    uint8_t  host;
    uint32_t time;
public:
    static Timestamp getNull();

    Timestamp(uint8_t net_, uint8_t host_, uint32_t time_) :  net(net_), host(host_), time(time_) {}
    Timestamp() : net(0), host(0), time(0) {}

    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(net, host, time);
        return bb;
    }
    std::string toString() const override;
    
    bool isNull() const {
        return net == 0 && host == 0 && time == 0;
    }

    inline auto operator<=>(const Timestamp& that) const {
        auto ret = this->time <=> that.time;
        if (ret == std::strong_ordering::equal) ret = this->net <=> that.net;
        if (ret == std::strong_ordering::equal) ret = this->host <=> that.host;
        return ret;
    }
    inline auto operator==(const Timestamp& that) const {
        return this->time == that.time && this->net == that.net && this->host == that.host;
    }
};

// NameRecord: TYPE = RECORD [CARDINAL];
// NullName: NameRecord = [1];
struct NameRecord : public Index<"ss", std::string> {
    static const constexpr uint16_t NULL_NAME = 1;

    bool isNull() const {
        return index == NULL_NAME;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        return Index::toString();
    }
};


// FTRecord: TYPE = RECORD [name: NameRecord, version: VersionStamp];
struct FTRecord : public ByteBuffer::Readable, public HasToString {
    NameRecord name;
    Timestamp version;

    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(name, version);
        return bb;
    }

    std::string toString() const override {
        return std_sprintf("%s#%s", version.toString(), name.toString());
    }
};
// FTIndex: TYPE = Table.Base RELATIVE POINTER [0..tLimit] TO FTRecord;
// FTNull: FTIndex = LAST[FTIndex];
// FTSelf: FTIndex = LAST[FTIndex] - 1;
struct FTIndex : public Index<"ft", FTRecord> {
    static const constexpr uint16_t FT_NULL = T_LIMIT;
    static const constexpr uint16_t FT_SELF = T_LIMIT - 1;

    bool isNull() const {
        return index == FT_NULL;
    }
    bool isSelf() const {
        return index == FT_SELF;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        if (isSelf()) return std_sprintf("%s-SELF", prefix);
        return Index::toString();
    }
};


// SGRecord: TYPE = RECORD [
//   file: FTIndex, base: CARDINAL,
//   pages: [0..256), extraPages: [0..64), class: SegClass];
struct SGRecord : public ByteBuffer::Readable, public HasToString {
	// SegClass: TYPE = {code, symbols, acMap, other};
	enum class SegClass {
		CODE, SYMBOLS, AC_MAP, OTHER,
	};

    FTIndex     file;
    uint16_t    base;
    uint16_t    pages;
    uint16_t    extraPages;
    SegClass    segClass;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};
// SGIndex: TYPE = Table.Base RELATIVE POINTER [0..tLimit] TO SGRecord;
// SGNull: SGIndex = LAST[SGIndex];
struct SGIndex : public Index<"sg", SGRecord> {
    static const constexpr uint16_t SG_NULL = T_LIMIT;

    bool isNull() const {
        return index == SG_NULL;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        return Index::toString();
    }
};


//ENRecord: TYPE = RECORD [
//   nEntries: CARDINAL, initialPC: ARRAY [0..0) OF PrincOps.BytePC];
struct ENRecord : public ByteBuffer::Readable, public HasToString {
    std::vector<uint16_t> initialPC;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};
//ENIndex: TYPE = Table.Base RELATIVE POINTER [0..tLimit] TO ENRecord;
//ENNull: ENIndex = LAST[ENIndex];
struct ENIndex : public Index<"en", ENRecord> {
    static const constexpr uint16_t EN_NULL = T_LIMIT;

    bool isNull() const {
        return index == EN_NULL;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        return Index::toString();
    }
};

//   CodeDesc: TYPE = RECORD [
//     sgi: SGIndex, offset, length: CARDINAL];
struct CodeDesc: public ByteBuffer::Readable, public HasToString {
    SGIndex  sgi;
    uint16_t offset;
    uint16_t length;

    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(sgi, offset, length);
        return bb;
    }

    std::string toString() const override {
        return std_sprintf("[%s  %5d  %5d]", sgi.toString(), offset, length);
    }
};

// LinkLocation: TYPE = {frame, code, dontcare};
enum class LinkLocation {
	FRAME, CODE, DONTCARE,
};
std::string toString(LinkLocation value);
// MTRecord: TYPE = --MACHINE DEPENDENT-- RECORD [
//     name: NameRecord,
//     file: FTIndex,
//     config: CTIndex,
//     code: CodeDesc,
//     sseg: SGIndex,
//     links: LFIndex,
//     linkLoc: LinkLocation,
//     namedInstance, initial: BOOLEAN, 
//     boundsChecks, nilChecks: BOOLEAN,
//     tableCompiled, residentFrame, crossJumped, packageable: BOOLEAN,
//     packed: BOOLEAN, linkspace: BOOLEAN,
//     spare: PACKED ARRAY [0..4) OF BOOLEAN,
//     framesize: [0..PrincOps.MaxFrameSize),
//     entries: ENIndex,
//     atoms: ATIndex];
struct MTRecord : public ByteBuffer::Readable, public HasToString {
    // LinkLocation: TYPE = {frame, code, dontcare};
    enum class LinkLocation {
        FRAME, CODE, DONTCARE,
    };

    NameRecord   name;
    FTIndex      file;
    uint16_t     config;
    CodeDesc     code;
    SGIndex      sseg;
    uint16_t     links;
    LinkLocation linkLoc;        //  01
    bool         namedInstance;  //  2
    bool         initial;        //  3
    bool         boundsChecks;   //  4
    bool         nilChecks;      //  5
    bool         tableCompiled;  //  6
    bool         residentFrame;  //  7
    bool         crossJumped;    //  8
    bool         packageable;    //  9
    bool         packed;         // 10
    bool         linkspace;      // 11
    bool         spare0;         // 12
    bool         spare1;         // 13
    bool         spare2;         // 14
    bool         spare3;         // 15
    uint16_t     frameSize;
    ENIndex      entries;
    uint16_t     atoms;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};
//MTIndex: TYPE = Table.Base RELATIVE POINTER [0..tLimit] TO MTRecord;
//MTNull: MTIndex = LAST[MTIndex];
struct MTIndex : public Index<"mt", MTRecord> {
    static const constexpr uint16_t MT_NULL = T_LIMIT;

    bool isNull() const {
        return index == MT_NULL;
    }
    std::string toString() const override {
        if (isNull()) return std_sprintf("%s-NULL", prefix);
        return Index::toString();
    }
};


struct BCD : public ByteBuffer::Readable {
    static constexpr uint16_t VersionID = 6103;

	uint16_t  versionIdent;
	Timestamp version;
	Timestamp creator;

    FTIndex   sourceFile;
    FTIndex   unpackagedFile;

    uint16_t  nConfigs;
	uint16_t  nModules;
	uint16_t  nImports;
	uint16_t  nExports;
	uint16_t  nPages;
	bool      definitions;
	bool      repackaged;
	bool      typeExported;
	bool      tableCompiled;
	uint16_t  spare4;
	uint16_t  firstDummy;
	uint16_t  nDummies;

    #undef BCD_TABLE
    #define BCD_TABLE(prefix) uint16_t prefix##Offset; uint16_t prefix##Limit;

    BCD_TABLE(ss)  // string table
    BCD_TABLE(ct)  // config table
    BCD_TABLE(mt)  // module table
    BCD_TABLE(imp) // import table
    BCD_TABLE(exp) // export table
    BCD_TABLE(en)  // entry table
    BCD_TABLE(sg)  // segment table
    BCD_TABLE(ft)  // file table
    BCD_TABLE(sp)  // space table
    BCD_TABLE(nt)  // name table
    BCD_TABLE(typ) // type table
    BCD_TABLE(tm)  // type map table
    BCD_TABLE(fp)  // frame pack table
    BCD_TABLE(lf)  // link fragment table
    BCD_TABLE(at)  // atom table
    BCD_TABLE(ap)  // atom print table

    std::map<uint16_t, std::string> ssTable;
    std::map<uint16_t, FTRecord>    ftTable;
    std::map<uint16_t, SGRecord>    sgTable;
    std::map<uint16_t, ENRecord>    enTable;
    std::map<uint16_t, MTRecord>    mtTable;

    ByteBuffer& read(ByteBuffer& bb) override;

    void dump();

    static uint16_t getIndex(int pos, int offset, int limit);

    void buildSSTable(ByteBuffer& bb);
    void buildFTTable(ByteBuffer& bb);
    void buildSGTable(ByteBuffer& bb);
    void buildENTable(ByteBuffer& bb);
    void buildMTTable(ByteBuffer& bb);
};
