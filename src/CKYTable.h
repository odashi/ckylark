#ifndef CKYLARK_CKY_TABLE_H_
#define CKYLARK_CKY_TABLE_H_

#include <stdexcept>

namespace Ckylark {

template <class T>
class CKYTable {

public:
    CKYTable(size_t num_words, size_t num_tags)
        : num_words_(num_words)
        , num_tags_(num_tags) {
    
        stride1_ = 0;
        while (static_cast<size_t>(1 << stride1_) < num_words_ + 1) {
            ++stride1_;
        }
        stride2_ = 0;
        while (static_cast<size_t>(1 << stride2_) < num_tags_) {
            ++stride2_;
        }
        stride1_ += stride2_;

        mem_ = new T[(1 << stride1_) * num_words];
    }
    
    ~CKYTable() {
        delete[] mem_;
    }

    inline const T & at(size_t begin, size_t end, size_t tag) const {
        if (begin >= num_words_ || end > num_words_ || end <= begin || tag >= num_tags_)
            throw std::runtime_error("CKYTable: invalid index");
        return mem_[(begin << stride1_) + (end << stride2_) + tag];
    }

    inline T & at(size_t begin, size_t end, size_t tag) {
        return const_cast<T &>(static_cast<const CKYTable &>(*this).at(begin, end, tag));
    }

    inline size_t numWords() const { return num_words_; }
    inline size_t numTags() const { return num_tags_; }

private:
    size_t num_words_;
    size_t num_tags_;
    size_t stride1_;
    size_t stride2_;
    T * mem_;

}; // class CKYTable

} // namespace Ckylark

#endif // CKYLARK_CKY_TABLE_H_
