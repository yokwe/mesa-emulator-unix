

#include <set>
#include <map>

#include "../util/Util.h"

#include "../util/ByteBuffer.h"


template <StringLiteral PREFIX, class T>
struct Index : public ByteBuffer::Readable, public HasToString {
    static inline const char* prefix = PREFIX;

    static inline std::set<Index*>       indexSet;
    static inline std::map<uint16_t, T*> valueMap;

    static void addIndex(Index* index) {
        if (indexSet.contains(index)) {
            // unexpected
            logger.error("Duplicate key");
            logger.error("  prefix  %s", prefix);
            logger.error("  index   %p  %d", index, index->index);
            ERROR()
        }
        // after index has assigned register to map
        indexSet.insert(index);
    }
    static void removeIndex(Index* reference) {
        if (indexSet.contains(reference)) {
            indexSet.erase(reference);
        }
    }
    static void addValue(uint16_t index, T* value) {
        if (valueMap.contains(index)) {
            // not expect this
            logger.error("Duplicate key");
            logger.error("  index   %d", index);
            ERROR()
        } else {
            valueMap[index] = value;
        }
    }
    static void clearMap() {
        indexSet.clear();
        valueMap.clear();
    }
    static void fillValue() {
        for(auto i = indexSet.begin(); i != indexSet.end(); i++) {
            Index* p = *i;
            auto index = p->index;
            if (index == -1) {
                logger.info("index = -1");
                continue;
            }
            if (valueMap.contains(index)) {
                p->value = valueMap[index];
            }
        }
    }
    static void dump() {
        constexpr auto is_string  = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;
        for(const auto& e: indexSet) {
            logger.info("dump  index  %s  %s", prefix, e->toString());
        }
        for(const auto& e: valueMap) {
            if constexpr (is_string) {
                logger.info("dump  value  %s  %d  %s", prefix, e.first, e.second);
            } else {
                logger.info("dump  value  %s  %d  %s", prefix, e.first, e.second->toString());
            }
        }
    }

    uint16_t index;
    T*       value;

    // default constructor
    Index() : ByteBuffer::Readable(), index(-1), value(0) {
        addIndex(this);
    }
    // copy constructor
    Index(const Index& that) {
        this->index = that.index;
        this->value = that.value;
        addIndex(this);
    }
    // move constructor
    Index(Index&& that) {
        this->index = that.index;
        this->value = that.value;
        addIndex(this);
    }
    ~Index() {
        removeIndex(this);
    }

    bool hasValue() {
        return value;
    }
    T* getValue() {
        return value;
    }
    operator uint16_t() {
        return index;
    }
    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(index);
        value = 0;
        return bb;
    }
    std::string toString() override {
        if (value == 0) return std_sprintf("%s-%d", prefix, index);
        constexpr auto is_string  = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;
        if constexpr(is_string) {
            return *value;
        } else {
            return value->toString();
        }
    }
};
