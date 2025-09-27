//
//
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Ethernet.h"

namespace xns::ethernet::host {

UINT48 Host::BROADCAST = Host{0xFFFF'FFFF'FFFFL, "BROADCAST"};
UINT48 Host::UNKNOWN   = Host{0, "UNKNOWN"};


}