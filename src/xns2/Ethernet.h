//
// Ethernet.h
//

#pragma once

#include "Type.h"

namespace xns::ethernet::host {

class Host : public UINT48 {
    static inline const char* group = "xns::ethernet::host::Host";
    Host(uint64_t value_, const char* name_) : UINT48(group, value_, name_) {
    }
public:
    Host() : UINT48("%12lX") {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    void toByteBuffer (ByteBuffer& bb) const {
        bb.write48(value);
    }

    static UINT48 BROADCAST;
    static UINT48 UNKNOWN;
};

}

