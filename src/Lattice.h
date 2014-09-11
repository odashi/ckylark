#ifndef AHCP_LATTICE_H_
#define AHCP_LATTICE_H_

#include <string>
#include <vector>

namespace AHCParser {

class Lattice {

    Lattice(const Lattice &) = delete;

public:
    struct Edge {
        std::string word;
        double score;
    }; // struct Edge

public:
    Lattice();
    ~Lattice();

    void addEdge(size_t begin, size_t end, const Edge & edge);

    const std::vector<Edge> * getEdgeList(size_t begin, size_t end) const;

    size_t numNodes() const { return edge_list_.size() + 1; }

private:
    std::vector<std::vector<std::vector<Edge> > > edge_list_; // [begin][end]{word}

}; // class Lattice

} // namespace AHCParser

#endif // AHCP_LATTICE_H_

