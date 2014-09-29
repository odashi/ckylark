#ifndef CKYLARK_LATTICE_H_
#define CKYLARK_LATTICE_H_

#include <string>
#include <vector>

namespace Ckylark {

/*
    this class treats only directed acyclic graph (DAG) shaped lattice.
*/

class Lattice {

    Lattice(const Lattice &) = delete;
    Lattice & operator=(const Lattice &) = delete;

public:
    struct Edge {
        std::string word;
        double score;
    }; // struct Edge

public:
    Lattice();
    ~Lattice();

    void addEdge(size_t begin, size_t end, const Edge & edge);

    const std::vector<Edge> & getEdgeList(size_t begin, size_t end) const;

    size_t numNodes() const { return edge_list_.size() + 1; }

private:
    std::vector<std::vector<std::vector<Edge> > > edge_list_; // [begin][end]{word}
    std::vector<Edge> dummy_edge_list_;

    size_t getEndPos(size_t begin, size_t end) const;

}; // class Lattice

} // namespace Ckylark

#endif // CKYLARK_LATTICE_H_

