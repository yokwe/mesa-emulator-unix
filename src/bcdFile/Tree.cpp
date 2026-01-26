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
// Tree.cpp
//

#include <string>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/ByteBuffer.h"

#include "Tree.h"

#undef  ENUM_NAME
#define ENUM_NAME(enum,name) {enum::name, #name},

//
// TreeLink
//
std::string TreeLink::toString(Tag value) {
    static std::map<Tag, std::string> map {
        ENUM_NAME(Tag, SUBTREE)
        ENUM_NAME(Tag, HASH)
        ENUM_NAME(Tag, SYMBOL)
        ENUM_NAME(Tag, LITERAL)
    };
    
    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
ByteBuffer& TreeLink::read(ByteBuffer& bb) {
    uint16_t u0;

    bb.read(u0);
    tag = (Tag)bitField(u0, 0, 1);
    switch(tag) {
    case Tag::SUBTREE:
    {
        SUBTREE value;
        value.read(u0);
        variant = value;
    }
        break;
    case Tag::HASH:
    {
        HASH value;
        value.read(u0);
        variant = value;
    }
        break;
    case Tag::SYMBOL:
    {
        SYMBOL value;
        value.read(u0);
        variant = value;
    }
        break;
    case Tag::LITERAL:
    {
        LITERAL value;
        value.read(u0);
        variant = value;
    }
        break;
    default:
        ERROR()
    }

    return bb;
}
std::string TreeLink::toString() const {
    std::string variantString = ::toStringVariant(variant);
    return std_sprintf("[%s  %s]", toString(tag), variantString);
}

//
// NodeName
//
std::string toString(NodeName value) {
    static std::map<NodeName, std::string> map {
        ENUM_NAME(NodeName, LIST)
        ENUM_NAME(NodeName, ITEM)

        // -- declarations
        ENUM_NAME(NodeName, DECL)
        ENUM_NAME(NodeName, TYPE_DECL)
        ENUM_NAME(NodeName, BASIC_TC)
        ENUM_NAME(NodeName, ENUMERATED_TC)
        ENUM_NAME(NodeName, RECORD_TC)
        ENUM_NAME(NodeName, MONITORED_TC)
        ENUM_NAME(NodeName, VARIANT_TC)
        ENUM_NAME(NodeName, REF_TC)
        ENUM_NAME(NodeName, POINTER_TC)
        ENUM_NAME(NodeName, LIST_TC)
        ENUM_NAME(NodeName, ARRAY_TC)
        ENUM_NAME(NodeName, ARRAYDESC_TC)
        ENUM_NAME(NodeName, SEQUENCE_TC)
        ENUM_NAME(NodeName, PROC_TC)
        ENUM_NAME(NodeName, PROCESS_TC)
        ENUM_NAME(NodeName, PORT_TC)
        ENUM_NAME(NodeName, SIGNAL_TC)
        ENUM_NAME(NodeName, ERROR_TC)
        ENUM_NAME(NodeName, PROGRAM_TC)
        ENUM_NAME(NodeName, ANY_TC)
        ENUM_NAME(NodeName, DEFINITION_TC)
        ENUM_NAME(NodeName, UNION_TC)
        ENUM_NAME(NodeName, RELATIVE_TC)
        ENUM_NAME(NodeName, SUBRANGE_TC)
        ENUM_NAME(NodeName, LONG_TC)
        ENUM_NAME(NodeName, OPAQUE_TC)
        ENUM_NAME(NodeName, ZONE_TC)
        ENUM_NAME(NodeName, LINK_TC)
        ENUM_NAME(NodeName, VAR_TC)
        ENUM_NAME(NodeName, IMPLICIT_TC)
        ENUM_NAME(NodeName, FRAME_TC)
        ENUM_NAME(NodeName, DISCRIM_TC)
        ENUM_NAME(NodeName, ENTRY)
        ENUM_NAME(NodeName, INTERNAL)
        ENUM_NAME(NodeName, UNIT)
        ENUM_NAME(NodeName, DIRITEM)
        ENUM_NAME(NodeName, MODULE)
        ENUM_NAME(NodeName, BODY)
        ENUM_NAME(NodeName, INLINE)
        ENUM_NAME(NodeName, LAMBDA)
        ENUM_NAME(NodeName, BLOCK)

        //-- statements
        ENUM_NAME(NodeName, ASSIGN)
        ENUM_NAME(NodeName, EXTRACT)
        ENUM_NAME(NodeName, IF)
        ENUM_NAME(NodeName, CASE)
        ENUM_NAME(NodeName, CASETEST)
        ENUM_NAME(NodeName, CASESWITCH)
        ENUM_NAME(NodeName, BIND)
        ENUM_NAME(NodeName, DO)
        ENUM_NAME(NodeName, FORSEQ)
        ENUM_NAME(NodeName, UPTHRU)
        ENUM_NAME(NodeName, DOWNTHRU)
        ENUM_NAME(NodeName, RETURN)
        ENUM_NAME(NodeName, RESULT)
        ENUM_NAME(NodeName, GOTO)
        ENUM_NAME(NodeName, EXIT)
        ENUM_NAME(NodeName, LOOP)
        ENUM_NAME(NodeName, FREE)
        ENUM_NAME(NodeName, RESUME)
        ENUM_NAME(NodeName, REJECT)
        ENUM_NAME(NodeName, CONTINUE)
        ENUM_NAME(NodeName, RETRY)
        ENUM_NAME(NodeName, CATCHMARK)
        ENUM_NAME(NodeName, RESTART)
        ENUM_NAME(NodeName, STOP)
        ENUM_NAME(NodeName, LOCK)
        ENUM_NAME(NodeName, WAIT)
        ENUM_NAME(NodeName, NOTIFY)
        ENUM_NAME(NodeName, BROADCAST)
        ENUM_NAME(NodeName, UNLOCK)
        ENUM_NAME(NodeName, NULL_)
        ENUM_NAME(NodeName, LABEL)
        ENUM_NAME(NodeName, OPEN)
        ENUM_NAME(NodeName, ENABLE)
        ENUM_NAME(NodeName, CATCH)
        ENUM_NAME(NodeName, DSK)
        ENUM_NAME(NodeName, LSK)
        ENUM_NAME(NodeName, XE)
        ENUM_NAME(NodeName, XF)
        ENUM_NAME(NodeName, SYSCALL)
        ENUM_NAME(NodeName, CHECKED)
        ENUM_NAME(NodeName, SPARES2)
        ENUM_NAME(NodeName, SPARES3)
        ENUM_NAME(NodeName, SUBST)
        ENUM_NAME(NodeName, CALL)
        ENUM_NAME(NodeName, PORTCALL)
        ENUM_NAME(NodeName, SIGNAL)
        ENUM_NAME(NodeName, ERROR)
        ENUM_NAME(NodeName, SYSERROR)
        ENUM_NAME(NodeName, XERROR)
        ENUM_NAME(NodeName, START)
        ENUM_NAME(NodeName, JOIN)

        // -- expressions
        ENUM_NAME(NodeName, APPLY)
        ENUM_NAME(NodeName, CALLX)
        ENUM_NAME(NodeName, PORTCALLX)
        ENUM_NAME(NodeName, SIGNALX)
        ENUM_NAME(NodeName, ERRORX)
        ENUM_NAME(NodeName, SYSERRORX)
        ENUM_NAME(NodeName, STARTX)
        ENUM_NAME(NodeName, FORK)
        ENUM_NAME(NodeName, JOINX)
        ENUM_NAME(NodeName, INDEX)
        ENUM_NAME(NodeName, DINDEX)
        ENUM_NAME(NodeName, SEQINDEX)
        ENUM_NAME(NodeName, RELOC)
        ENUM_NAME(NodeName, CONSTRUCT)
        ENUM_NAME(NodeName, UNION)
        ENUM_NAME(NodeName, ROWCONS)
        ENUM_NAME(NodeName, SEQUENCE)
        ENUM_NAME(NodeName, LISTCONS)
        ENUM_NAME(NodeName, SUBSTX)
        ENUM_NAME(NodeName, IFX)
        ENUM_NAME(NodeName, CASEX)
        ENUM_NAME(NodeName, BINDX)
        ENUM_NAME(NodeName, ASSIGNX)
        ENUM_NAME(NodeName, EXTRACTX)
        ENUM_NAME(NodeName, OR)
        ENUM_NAME(NodeName, AND)
        ENUM_NAME(NodeName, RELE)
        ENUM_NAME(NodeName, RELN)
        ENUM_NAME(NodeName, RELL)
        ENUM_NAME(NodeName, RELGE)
        ENUM_NAME(NodeName, RELG)
        ENUM_NAME(NodeName, RELLE)
        ENUM_NAME(NodeName, IN)
        ENUM_NAME(NodeName, NOTIN)
        ENUM_NAME(NodeName, PLUS)
        ENUM_NAME(NodeName, MINUS)
        ENUM_NAME(NodeName, TIMES)
        ENUM_NAME(NodeName, DIV)
        ENUM_NAME(NodeName, MOD)
        ENUM_NAME(NodeName, DOT)
        ENUM_NAME(NodeName, CDOT)
        ENUM_NAME(NodeName, DOLLAR)
        ENUM_NAME(NodeName, CREATE)
        ENUM_NAME(NodeName, NOT)
        ENUM_NAME(NodeName, UMINUS)
        ENUM_NAME(NodeName, ADDR)
        ENUM_NAME(NodeName, UPARROW)
        ENUM_NAME(NodeName, MIN)
        ENUM_NAME(NodeName, MAX)
        ENUM_NAME(NodeName, LENGTHEN)
        ENUM_NAME(NodeName, ABS)
        ENUM_NAME(NodeName, ALL)
        ENUM_NAME(NodeName, SIZE)
        ENUM_NAME(NodeName, FIRST)
        ENUM_NAME(NodeName, LAST)
        ENUM_NAME(NodeName, PRED)
        ENUM_NAME(NodeName, SUCC)
        ENUM_NAME(NodeName, ARRAYDESC)
        ENUM_NAME(NodeName, LENGTH)
        ENUM_NAME(NodeName, BASE)
        ENUM_NAME(NodeName, LOOPHOLE)
        ENUM_NAME(NodeName, NIL)
        ENUM_NAME(NodeName, NEW)
        ENUM_NAME(NodeName, VOID)
        ENUM_NAME(NodeName, CLIT)
        ENUM_NAME(NodeName, LLIT)
        ENUM_NAME(NodeName, CAST)
        ENUM_NAME(NodeName, CHECK)
        ENUM_NAME(NodeName, FLOAT)
        ENUM_NAME(NodeName, PAD)
        ENUM_NAME(NodeName, CHOP)
        ENUM_NAME(NodeName, SAFEN)
        ENUM_NAME(NodeName, SYSCALLX)
        ENUM_NAME(NodeName, NARROW)
        ENUM_NAME(NodeName, ISTYPE)
        ENUM_NAME(NodeName, OPENX)
        ENUM_NAME(NodeName, MWCONST)
        ENUM_NAME(NodeName, CONS)
        ENUM_NAME(NodeName, ATOM)
        ENUM_NAME(NodeName, TYPECODE)
        ENUM_NAME(NodeName, STRINGINIT)
        ENUM_NAME(NodeName, TEXTLIT)
        ENUM_NAME(NodeName, SIGNALINIT)
        ENUM_NAME(NodeName, PROCINIT)
        ENUM_NAME(NodeName, INTOO)
        ENUM_NAME(NodeName, INTOC)
        ENUM_NAME(NodeName, INTCO)
        ENUM_NAME(NodeName, INTCC)
        ENUM_NAME(NodeName, THREAD)
        ENUM_NAME(NodeName, NONE)
        ENUM_NAME(NodeName, EXLIST)
        ENUM_NAME(NodeName, INITLIST)
        ENUM_NAME(NodeName, DITEM)
        ENUM_NAME(NodeName, SHORTEN)
        ENUM_NAME(NodeName, SELF)
        ENUM_NAME(NodeName, GCRT)
        ENUM_NAME(NodeName, PROCCHECK)
        ENUM_NAME(NodeName, ORD)
        ENUM_NAME(NodeName, VAL)
        ENUM_NAME(NodeName, MERGECONS)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

//
// TreeNode
//
ByteBuffer& TreeNode::read(ByteBuffer& bb) {
    uint16_t u0;
    bb.read(u0, info);

    free   = bitField(u0,  0);
    name   = (NodeName)bitField(u0,  1, 8);
    attr1  = bitField(u0,  9);
    attr2  = bitField(u0, 10);
    attr3  = bitField(u0, 11);
    shared = bitField(u0, 12);
    nSons  = bitField(u0, 13, 15);

    for(int i = 0; i < nSons; i++) {
        TreeLink value;
        bb.read(value);
        son.push_back(value);
    }

    return bb;
}
std::string TreeNode::toString() const {
    std::string flags = std_sprintf("[%s%s%s%s%s]",
        free   ? "F" : "_",
        shared ? "S" : "_",
        attr1  ? "1" : "_",
        attr2  ? "2" : "_",
        attr3  ? "3" : "_");
    std::string sonString;
    for(const auto& e: son) {
        sonString += std_sprintf(" %s", e.toString());
    }
    if (!sonString.empty()) sonString = sonString.substr(1);
    return std_sprintf("[%s  %-8s  %5d  (%d)[%s]]",
        flags, ::toString(name), info, nSons, sonString);
}
