#ifndef CKYLARK_LATTICE_LOADER_H_
#define CKYLARK_LATTICE_LOADER_H_

#include "Lattice.h"

#include <istream>
#include <memory>
#include <stdexcept>

namespace Ckylark {

class LatticeLoader {

public:
    LatticeLoader() : input_(nullptr) {}
    virtual ~LatticeLoader() {}

    void setInput(std::shared_ptr<std::istream> input) { input_ = input; }

    virtual std::shared_ptr<Lattice> getNextData() = 0;

protected:
    std::shared_ptr<std::istream> getInput() {
        if (input_ == nullptr) throw std::runtime_error("LatticeLoader: input is not set");
        return input_;
    }

private:
    std::shared_ptr<std::istream> input_;

}; // class LatticeLoader

} // namespace Ckylark

#endif // CKYLARK_LATTICE_LOADER_H_

