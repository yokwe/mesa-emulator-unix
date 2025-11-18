
#include <cstdint>
#include <string>

#include "../util/ByteBuffer.h"

class Timestamp : public ByteBuffer::Readable {
    uint8_t  net;
    uint8_t  host;
    uint32_t time;

    std::string string;
public:
    static Timestamp getNull();

    Timestamp(uint8_t net_, uint8_t host_, uint32_t time_, const std::string& name_) :
        net(net_), host(host_), time(time_), string(name_) {}
    Timestamp() : net(0), host(0), time(0), string("#NULL#") {}

    ByteBuffer& read(ByteBuffer& bb) override;

    std::string toString() {
        return string;
    }
    
    bool isNull() const {
        return net == 0 && host == 0 && time == 0;
    }

    inline auto operator<=>(const Timestamp& that) const {
        return this->string <=> that.string;
    }
    inline auto operator==(const Timestamp& that) const {
        return this->string == that.string;
    }
};
