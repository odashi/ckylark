#ifndef AHCP_PLF_LATTICE_LOADER_H_
#define AHCP_PLF_LATTICE_LOADER_H_

#include "LatticeLoader.h"

namespace AHCParser {

class PLFLatticeLoader : public LatticeLoader {

public:
    PLFLatticeLoader();
    ~PLFLatticeLoader();

    std::shared_ptr<Lattice> getNextData();

private:

}; // class PLFLatticeLoader

} // namespace AHCParser

#endif // AHCP_PLF_LATTICE_LOADER_H_

