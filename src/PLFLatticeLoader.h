#ifndef CKYLARK_PLF_LATTICE_LOADER_H_
#define CKYLARK_PLF_LATTICE_LOADER_H_

#include "LatticeLoader.h"

namespace Ckylark {

class PLFLatticeLoader : public LatticeLoader {

public:
    PLFLatticeLoader();
    ~PLFLatticeLoader();

    std::shared_ptr<Lattice> getNextData();

private:

}; // class PLFLatticeLoader

} // namespace Ckylark

#endif // CKYLARK_PLF_LATTICE_LOADER_H_

