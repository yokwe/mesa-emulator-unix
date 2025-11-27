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
// Tree.h
//

#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "../util/Util.h"

#include "Symbols.h"
#include "SymbolsIndex.h"

#include "LTRecord.h"


//Link: TYPE = RECORD [
//  SELECT tag(0:0..1): * FROM
//    subtree => [index(0:2..15): Tree.Index],
//    hash => [index(0:2..15): Symbols.HTIndex],
//    symbol => [index(0:2..15): Symbols.ISEIndex],
//    literal => [info(0:2..15): Literals.LitRecord]
//    ENDCASE];
struct TreeLink : public ByteBuffer::Readable, public HasToString {
    enum class Tag : uint16_t {
        ENUM_VALUE(Tag, SUBTREE)
        ENUM_VALUE(Tag, HASH)
        ENUM_VALUE(Tag, SYMBOL)
        ENUM_VALUE(Tag, LITERAL)
    };
    static std::string toString(Tag);

    struct SUBTREE : public HasToString {
        TreeIndex index;

        void read(uint16_t u0) {
            index.index(bitField(u0, 2, 15));
        }
        std::string toString() const {
            return std_sprintf("[%s]", index.Index::toString());
        }
    };
    struct HASH : public HasToString {
        HTIndex index;

        void read(uint16_t u0) {
            index.index(bitField(u0, 2, 15));
        }
        std::string toString() const {
            return std_sprintf("[%s]", index.Index::toString());
        }
    };
    struct SYMBOL : public HasToString {
        SEIndex index;

        void read(uint16_t u0) {
            index.index(bitField(u0, 2, 15));
        }
        std::string toString() const {
            return std_sprintf("[%s]", index.Index::toString());
        }
    };
    struct LITERAL : public HasToString {
        LitRecord index;

        void read(uint16_t u0) {
            // need to align bit 0 position of LitRecord
            index.read(bitField(u0, 2, 15) << 2);
        }
        std::string toString() const {
            return std_sprintf("[%s]", index.toString());
        }
    };

    Tag tag;
    std::variant<SUBTREE, HASH, SYMBOL, LITERAL> value;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};

// NodeName: TYPE = {
//     -- general tree constructors
//     list, item,
//
//     -- declarations
//     decl, typedecl,
//     basicTC, enumeratedTC, recordTC, monitoredTC, variantTC,
//     refTC, pointerTC, listTC, arrayTC, arraydescTC, sequenceTC,
//     procTC, processTC, portTC, signalTC, errorTC, programTC,
//     anyTC, definitionTC, unionTC, relativeTC,
//     subrangeTC, longTC, opaqueTC, zoneTC, linkTC, varTC,
//     implicitTC, frameTC, discrimTC,
//     entry, internal,
//     unit, diritem, module, body, inline, lambda, block,
//
//     -- statements
//     assign, extract,
//     if,
//     case, casetest, caseswitch,
//     bind,
//     do, forseq, upthru, downthru,
//     return, result,
//     goto, exit, loop,
//     free,
//     resume, reject, continue, retry, catchmark,
//     restart, stop,
//     lock, wait, notify, broadcast, unlock,
//     null,
//     label,
//     open,
//     enable, catch,
//     dsk, lsk, xe, xf,
//     syscall, checked, spareS2, spareS3,
//     subst, call, portcall, signal, error, syserror, xerror,
//     start, join,
//
//     -- expressions
//     apply,
//     callx, portcallx, signalx, errorx, syserrorx, startx, fork, joinx,
//     index, dindex, seqindex, reloc,
//     construct, union, rowcons, sequence, listcons,
//     substx,
//     ifx, casex, bindx,
//     assignx, extractx,
//     or, and,
//     relE, relN, relL, relGE, relG, relLE, in, notin,
//     plus, minus, times, div, mod,
//     dot, cdot, dollar,
//     create,
//     not,
//     uminus,
//     addr,
//     uparrow,
//     min, max, lengthen, abs, all,
//     size, first, last, pred, succ,
//     arraydesc, length, base,
//     loophole,
//     nil,
//     new,
//     void,
//     clit, llit,
//     cast, check, float, pad, chop, safen,
//     syscallx, narrow, istype,
//     openx,
//     mwconst, cons,
//     atom, typecode,
//     stringinit, textlit, signalinit, procinit,
//     intOO, intOC, intCO, intCC,
//
//     thread,
//     none,
//
//     exlist,
//     initlist,
//     ditem,
//
//     shorten,
//     self,
//     gcrt,
//     proccheck,
//
//     ord,
//     val,
//
//     mergecons};
enum class NodeName {
    // -- general tree constructors
    ENUM_VALUE(NodeName, LIST)
    ENUM_VALUE(NodeName, ITEM)

    // -- declarations
    ENUM_VALUE(NodeName, DECL)
    ENUM_VALUE(NodeName, TYPE_DECL)
    ENUM_VALUE(NodeName, BASIC_TC)
    ENUM_VALUE(NodeName, ENUMERATED_TC)
    ENUM_VALUE(NodeName, RECORD_TC)
    ENUM_VALUE(NodeName, MONITORED_TC)
    ENUM_VALUE(NodeName, VARIANT_TC)
    ENUM_VALUE(NodeName, REF_TC)
    ENUM_VALUE(NodeName, POINTER_TC)
    ENUM_VALUE(NodeName, LIST_TC)
    ENUM_VALUE(NodeName, ARRAY_TC)
    ENUM_VALUE(NodeName, ARRAYDESC_TC)
    ENUM_VALUE(NodeName, SEQUENCE_TC)
    ENUM_VALUE(NodeName, PROC_TC)
    ENUM_VALUE(NodeName, PROCESS_TC)
    ENUM_VALUE(NodeName, PORT_TC)
    ENUM_VALUE(NodeName, SIGNAL_TC)
    ENUM_VALUE(NodeName, ERROR_TC)
    ENUM_VALUE(NodeName, PROGRAM_TC)
    ENUM_VALUE(NodeName, ANY_TC)
    ENUM_VALUE(NodeName, DEFINITION_TC)
    ENUM_VALUE(NodeName, UNION_TC)
    ENUM_VALUE(NodeName, RELATIVE_TC)
    ENUM_VALUE(NodeName, SUBRANGE_TC)
    ENUM_VALUE(NodeName, LONG_TC)
    ENUM_VALUE(NodeName, OPAQUE_TC)
    ENUM_VALUE(NodeName, ZONE_TC)
    ENUM_VALUE(NodeName, LINK_TC)
    ENUM_VALUE(NodeName, VAR_TC)
    ENUM_VALUE(NodeName, IMPLICIT_TC)
    ENUM_VALUE(NodeName, FRAME_TC)
    ENUM_VALUE(NodeName, DISCRIM_TC)
    ENUM_VALUE(NodeName, ENTRY)
    ENUM_VALUE(NodeName, INTERNAL)
    ENUM_VALUE(NodeName, UNIT)
    ENUM_VALUE(NodeName, DIRITEM)
    ENUM_VALUE(NodeName, MODULE)
    ENUM_VALUE(NodeName, BODY)
    ENUM_VALUE(NodeName, INLINE)
    ENUM_VALUE(NodeName, LAMBDA)
    ENUM_VALUE(NodeName, BLOCK)

    //-- statements
    ENUM_VALUE(NodeName, ASSIGN)
    ENUM_VALUE(NodeName, EXTRACT)
    ENUM_VALUE(NodeName, IF)
    ENUM_VALUE(NodeName, CASE)
    ENUM_VALUE(NodeName, CASETEST)
    ENUM_VALUE(NodeName, CASESWITCH)
    ENUM_VALUE(NodeName, BIND)
    ENUM_VALUE(NodeName, DO)
    ENUM_VALUE(NodeName, FORSEQ)
    ENUM_VALUE(NodeName, UPTHRU)
    ENUM_VALUE(NodeName, DOWNTHRU)
    ENUM_VALUE(NodeName, RETURN)
    ENUM_VALUE(NodeName, RESULT)
    ENUM_VALUE(NodeName, GOTO)
    ENUM_VALUE(NodeName, EXIT)
    ENUM_VALUE(NodeName, LOOP)
    ENUM_VALUE(NodeName, FREE)
    ENUM_VALUE(NodeName, RESUME)
    ENUM_VALUE(NodeName, REJECT)
    ENUM_VALUE(NodeName, CONTINUE)
    ENUM_VALUE(NodeName, RETRY)
    ENUM_VALUE(NodeName, CATCHMARK)
    ENUM_VALUE(NodeName, RESTART)
    ENUM_VALUE(NodeName, STOP)
    ENUM_VALUE(NodeName, LOCK)
    ENUM_VALUE(NodeName, WAIT)
    ENUM_VALUE(NodeName, NOTIFY)
    ENUM_VALUE(NodeName, BROADCAST)
    ENUM_VALUE(NodeName, UNLOCK)
    ENUM_VALUE(NodeName, NULL_)
    ENUM_VALUE(NodeName, LABEL)
    ENUM_VALUE(NodeName, OPEN)
    ENUM_VALUE(NodeName, ENABLE)
    ENUM_VALUE(NodeName, CATCH)
    ENUM_VALUE(NodeName, DSK)
    ENUM_VALUE(NodeName, LSK)
    ENUM_VALUE(NodeName, XE)
    ENUM_VALUE(NodeName, XF)
    ENUM_VALUE(NodeName, SYSCALL)
    ENUM_VALUE(NodeName, CHECKED)
    ENUM_VALUE(NodeName, SPARES2)
    ENUM_VALUE(NodeName, SPARES3)
    ENUM_VALUE(NodeName, SUBST)
    ENUM_VALUE(NodeName, CALL)
    ENUM_VALUE(NodeName, PORTCALL)
    ENUM_VALUE(NodeName, SIGNAL)
    ENUM_VALUE(NodeName, ERROR)
    ENUM_VALUE(NodeName, SYSERROR)
    ENUM_VALUE(NodeName, XERROR)
    ENUM_VALUE(NodeName, START)
    ENUM_VALUE(NodeName, JOIN)

    // -- expressions
    ENUM_VALUE(NodeName, APPLY)
    ENUM_VALUE(NodeName, CALLX)
    ENUM_VALUE(NodeName, PORTCALLX)
    ENUM_VALUE(NodeName, SIGNALX)
    ENUM_VALUE(NodeName, ERRORX)
    ENUM_VALUE(NodeName, SYSERRORX)
    ENUM_VALUE(NodeName, STARTX)
    ENUM_VALUE(NodeName, FORK)
    ENUM_VALUE(NodeName, JOINX)
    ENUM_VALUE(NodeName, INDEX)
    ENUM_VALUE(NodeName, DINDEX)
    ENUM_VALUE(NodeName, SEQINDEX)
    ENUM_VALUE(NodeName, RELOC)
    ENUM_VALUE(NodeName, CONSTRUCT)
    ENUM_VALUE(NodeName, UNION)
    ENUM_VALUE(NodeName, ROWCONS)
    ENUM_VALUE(NodeName, SEQUENCE)
    ENUM_VALUE(NodeName, LISTCONS)
    ENUM_VALUE(NodeName, SUBSTX)
    ENUM_VALUE(NodeName, IFX)
    ENUM_VALUE(NodeName, CASEX)
    ENUM_VALUE(NodeName, BINDX)
    ENUM_VALUE(NodeName, ASSIGNX)
    ENUM_VALUE(NodeName, EXTRACTX)
    ENUM_VALUE(NodeName, OR)
    ENUM_VALUE(NodeName, AND)
    ENUM_VALUE(NodeName, RELE)
    ENUM_VALUE(NodeName, RELN)
    ENUM_VALUE(NodeName, RELL)
    ENUM_VALUE(NodeName, RELGE)
    ENUM_VALUE(NodeName, RELG)
    ENUM_VALUE(NodeName, RELLE)
    ENUM_VALUE(NodeName, IN)
    ENUM_VALUE(NodeName, NOTIN)
    ENUM_VALUE(NodeName, PLUS)
    ENUM_VALUE(NodeName, MINUS)
    ENUM_VALUE(NodeName, TIMES)
    ENUM_VALUE(NodeName, DIV)
    ENUM_VALUE(NodeName, MOD)
    ENUM_VALUE(NodeName, DOT)
    ENUM_VALUE(NodeName, CDOT)
    ENUM_VALUE(NodeName, DOLLAR)
    ENUM_VALUE(NodeName, CREATE)
    ENUM_VALUE(NodeName, NOT)
    ENUM_VALUE(NodeName, UMINUS)
    ENUM_VALUE(NodeName, ADDR)
    ENUM_VALUE(NodeName, UPARROW)
    ENUM_VALUE(NodeName, MIN)
    ENUM_VALUE(NodeName, MAX)
    ENUM_VALUE(NodeName, LENGTHEN)
    ENUM_VALUE(NodeName, ABS)
    ENUM_VALUE(NodeName, ALL)
    ENUM_VALUE(NodeName, SIZE)
    ENUM_VALUE(NodeName, FIRST)
    ENUM_VALUE(NodeName, LAST)
    ENUM_VALUE(NodeName, PRED)
    ENUM_VALUE(NodeName, SUCC)
    ENUM_VALUE(NodeName, ARRAYDESC)
    ENUM_VALUE(NodeName, LENGTH)
    ENUM_VALUE(NodeName, BASE)
    ENUM_VALUE(NodeName, LOOPHOLE)
    ENUM_VALUE(NodeName, NIL)
    ENUM_VALUE(NodeName, NEW)
    ENUM_VALUE(NodeName, VOID)
    ENUM_VALUE(NodeName, CLIT)
    ENUM_VALUE(NodeName, LLIT)
    ENUM_VALUE(NodeName, CAST)
    ENUM_VALUE(NodeName, CHECK)
    ENUM_VALUE(NodeName, FLOAT)
    ENUM_VALUE(NodeName, PAD)
    ENUM_VALUE(NodeName, CHOP)
    ENUM_VALUE(NodeName, SAFEN)
    ENUM_VALUE(NodeName, SYSCALLX)
    ENUM_VALUE(NodeName, NARROW)
    ENUM_VALUE(NodeName, ISTYPE)
    ENUM_VALUE(NodeName, OPENX)
    ENUM_VALUE(NodeName, MWCONST)
    ENUM_VALUE(NodeName, CONS)
    ENUM_VALUE(NodeName, ATOM)
    ENUM_VALUE(NodeName, TYPECODE)
    ENUM_VALUE(NodeName, STRINGINIT)
    ENUM_VALUE(NodeName, TEXTLIT)
    ENUM_VALUE(NodeName, SIGNALINIT)
    ENUM_VALUE(NodeName, PROCINIT)
    ENUM_VALUE(NodeName, INTOO)
    ENUM_VALUE(NodeName, INTOC)
    ENUM_VALUE(NodeName, INTCO)
    ENUM_VALUE(NodeName, INTCC)
    ENUM_VALUE(NodeName, THREAD)
    ENUM_VALUE(NodeName, NONE)
    ENUM_VALUE(NodeName, EXLIST)
    ENUM_VALUE(NodeName, INITLIST)
    ENUM_VALUE(NodeName, DITEM)
    ENUM_VALUE(NodeName, SHORTEN)
    ENUM_VALUE(NodeName, SELF)
    ENUM_VALUE(NodeName, GCRT)
    ENUM_VALUE(NodeName, PROCCHECK)
    ENUM_VALUE(NodeName, ORD)
    ENUM_VALUE(NodeName, VAL)
    ENUM_VALUE(NodeName, MERGECONS)
};
std::string toString(NodeName);

//Info: TYPE [SIZE[CARDINAL]];

//Node: TYPE = MACHINE DEPENDENT RECORD [
//  free (0: 0..0): BOOLEAN,    -- reserved for allocator
//  name (0: 1..8): NodeName,
//  attr1 (0: 9..9), attr2 (0: 10..10), attr3 (0: 11..11): BOOLEAN,
//  shared (0: 12..12): BOOLEAN,
//  nSons (0: 13..15): [0..MaxNSons],
//  info (1): Info,
//  son (2): ARRAY [1..1) OF Link];
struct TreeNode : public ByteBuffer::Readable, public HasToString {
    bool     free;
    NodeName name;
    bool     attr1;
    bool     attr2;
    bool     attr3;
    bool     shared;
    uint16_t nSons;
    uint16_t info;
    std::vector<TreeLink> son;

    ByteBuffer& read(ByteBuffer& bb) override;
    std::string toString() const override;
};
