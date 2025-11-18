

#include <set>
#include <map>

#include "../util/Util.h"

#include "../util/ByteBuffer.h"


template <StringLiteral PREFIX, class T>
struct Index : public ByteBuffer::Readable, public HasToString {
    static inline const char* prefix = PREFIX;

    static inline std::set<Index*>      indexSet;

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
    static void clearMap() {
        indexSet.clear();
    }
    static void setValue(std::map<uint16_t, T>& valueMap) {
        for(auto i = indexSet.begin(); i != indexSet.end(); i++) {
            Index* p = *i;
            if (p->noIndex) continue;
            auto index = p->index;
            if (valueMap.contains(index)) {
                p->value   = valueMap[index];
                p->noValue = false;
            }
        }
    }
    static void dump() {
        for(const auto& e: indexSet) {
            if (e->noIndex) continue;
            logger.info("dump  index  %s  %s", prefix, e->toString());
        }
    }

    uint16_t index;
    T        value;
    bool     noIndex;
    bool     noValue;

    // default constructor
    Index() : ByteBuffer::Readable(), index(55555), value(), noIndex(true), noValue(true) {
        addIndex(this);
    }
    // copy constructor
    Index(const Index& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
        this->noValue = that.noValue;
       addIndex(this);
    }
    // move constructor
    Index(Index&& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
        this->noValue = that.noValue;
        addIndex(this);
    }
    ~Index() {
        removeIndex(this);
    }

    Index& operator=(const Index& that) {
        this->index   = that.index;
        this->value   = that.value;
        this->noIndex = that.noIndex;
        this->noValue = that.noValue;
        return *this;
    }

    const T& getValue() {
        if (!noValue) return value;
        logger.error("index has no value");
        logger.error("  %s-%d", prefix, index);
        ERROR()
    }
    operator uint16_t() {
        return index;
    }
    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(index);
        noIndex = false;
        return bb;
    }
    std::string toString() const override {
        if (noValue) return std_sprintf("%s-%d", prefix, index);
        constexpr auto is_string  = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;
        if constexpr(is_string) {
            return value;
        } else {
            return value.toString();
        }
    }
};
