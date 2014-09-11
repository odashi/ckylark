#include "Lattice.h"

#include <stdexcept>

using namespace std;

namespace AHCParser {

Lattice::Lattice() {}

Lattice::~Lattice() {}

void Lattice::addEdge(size_t begin, size_t end, const Lattice::Edge & edge) {
    if (end <= begin) throw runtime_error("Lattice: invalid node ID");

    if (begin >= edge_list_.size()) {
        edge_list_.resize(begin + 1);
    }
    if (end >= edge_list_[begin].size()) {
        edge_list_[begin].resize(end + 1);
    }

    for (Edge & parallel : edge_list_[begin][end]) {
        if (edge.word == parallel.word) throw runtime_error("Lattice: duplicated edges");
    }

    edge_list_[begin][end].push_back(edge);
}

const vector<Lattice::Edge> * Lattice::getEdgeList(size_t begin, size_t end) const {
    if (end <= begin) throw runtime_error("Lattice: invalid node ID");
    
    if (begin >= edge_list_.size()) return nullptr;
    if (end >= edge_list_[begin].size()) return nullptr;

    return &edge_list_[begin][end];
}

} // namespace AHCParser

