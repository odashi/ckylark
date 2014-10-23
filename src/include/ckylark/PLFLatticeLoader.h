#ifndef CKYLARK_PLF_LATTICE_LOADER_H_
#define CKYLARK_PLF_LATTICE_LOADER_H_

#include <ckylark/LatticeLoader.h>

namespace Ckylark {

class PLFLatticeLoader : public LatticeLoader {
    
    PLFLatticeLoader(const PLFLatticeLoader &) = delete;
    PLFLatticeLoader & operator =(const PLFLatticeLoader &) = delete;

public:
    PLFLatticeLoader();
    ~PLFLatticeLoader();

    std::shared_ptr<Lattice> getNextData();

private:

}; // class PLFLatticeLoader

} // namespace Ckylark

#endif // CKYLARK_PLF_LATTICE_LOADER_H_

