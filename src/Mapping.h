#ifndef AHCP_MAPPING_H_
#define AHCP_MAPPING_H_

#include "TagSet.h"

#include <vector>

namespace AHCParser {

class Mapping {

public:
    Mapping(const TagSet & tag_set, int coarse_level, int fine_level);
    ~Mapping();

    inline int getCoarsePos(int tag, int subtag) const {
        return coarse_map_[tag][subtag];
    }
    inline int getFinePos(int tag, int subtag) const {
        return fine_map_[tag][subtag];
    }
    inline int getFineToCoarseMap(int tag, int fine_subtag) const {
        return f2c_map_[tag][fine_subtag];
    }
    inline const std::vector<int> & getCoarseToFineMaps(int tag, int coarse_subtag) const {
        return c2f_map_[tag][coarse_subtag];
    }

    inline size_t getNumCoarsePos() const { return nmap_coarse_; }
    inline size_t getNumFinePos() const { return nmap_fine_; }

private:
    const TagSet & tag_set_;
    int coarse_level_;
    int fine_level_;
    size_t nmap_coarse_;
    size_t nmap_fine_;
    std::vector<std::vector<int> > coarse_map_; // [cat_coarse][subcat_coarse] = coarse_pos
    std::vector<std::vector<int> > fine_map_; // [cat_fine][subcat_fine] = fine_pos
    std::vector<std::vector<int> > f2c_map_; // [cat_fine][subcat_fine] = subcat_coarse
    std::vector<std::vector<std::vector<int> > > c2f_map_; // [cat_coarse][subcat_coarse] = [subcat_fine]

}; // class Mapping

} // namespace AHCParser

#endif // AHCP_MAPPING_H_

