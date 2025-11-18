

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Timestamp.h"

Timestamp Timestamp::getNull() {
    static Timestamp ret;
    return ret;
}
static std::string toTimestamp(uint32_t unixTime) {
	time_t temp = unixTime;
    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%04d%02d%02d#%02d%02d%02d", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
ByteBuffer& Timestamp::read(ByteBuffer& bb) {
    bb.read(net, host, time);
    string = isNull() ? "#NULL" : std_sprintf("%s#%03d#%03d", toTimestamp(Util::toUnixTime(time)), net, host);
    return bb;
}
