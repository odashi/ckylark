#include <ckylark/Mapping.h>

using namespace std;

namespace Ckylark {

Mapping::Mapping(const TagSet & tag_set, int coarse_level, int fine_level)
    : tag_set_(tag_set)
    , coarse_level_(coarse_level)
    , fine_level_(fine_level)
    , coarse_map_()
    , fine_map_()
    , f2c_map_()
    , c2f_map_() {

    size_t nc = tag_set_.numTags();
    nmap_fine_ = 0;
    nmap_coarse_ = 0;

    for (size_t i = 0; i < nc; ++i) {
        size_t fine_nsc = tag_set_.numSubtags(i, fine_level);
        size_t coarse_nsc = tag_set_.numSubtags(i, fine_level);

        fine_map_.push_back(vector<int>(fine_nsc));
        for (int & x : fine_map_[i]) {
            x = nmap_fine_++;
        }

        coarse_map_.push_back(vector<int>(coarse_nsc));
        for (int & x : coarse_map_[i]) {
            x = nmap_coarse_++;
        }

        f2c_map_.push_back(vector<int>(fine_nsc, -1));
        c2f_map_.push_back(vector<vector<int> >(coarse_nsc, vector<int>()));
        auto & tree = tag_set_.getSubtagTree(i);
        for (auto & subtree : tree.getSubtrees(coarse_level, fine_level)) {
            int coarse = subtree->value();
            for (int fine : subtree->getLeaves()) {
                f2c_map_[i][fine] = coarse;
                c2f_map_[i][coarse].push_back(fine);
            }
        }

    }
}

Mapping::~Mapping() {}

} // namespace Ckylark

