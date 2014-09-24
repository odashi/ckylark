#ifndef CKYLARK_TREE_H_
#define CKYLARK_TREE_H_

#include <stdexcept>
#include <memory>
#include <vector>

namespace Ckylark {

template <class T>
class Tree {

    Tree() = delete;

public:
    explicit Tree(const T & value) : value_(value), parent_(nullptr), children_() {}

    Tree(const Tree & src) : value_(src.value_), parent_(nullptr), children_() {
        for (const Tree<T> * child : src.children_) {
            addChild(child->copyTree());
        }
    }

    ~Tree() {
        for (Tree<T> * child : children_) {
            delete child;
        }
    }

    inline bool isRoot() const { return !parent_; }
    inline bool isLeaf() const { return children_.size() == 0; }
    inline size_t numChildren() const { return children_.size(); }

    inline void addChild(Tree * child) {
        if (!child->isRoot()) throw std::runtime_error("Tree: already has parent.");
        children_.push_back(child);
        child->parent_ = this;
    }

    inline const T & value() const { return value_; }
    inline T & value() { return value_; }
    
    inline const Tree<T> & parent() const {
        if (isRoot()) throw std::runtime_error("Tree: not has parent.");
        return *parent_;
    }
    inline const Tree<T> & child(size_t index) const {
        if (index >= children_.size()) throw std::runtime_error("Tree: invalid child index");
        return *children_[index];
    }
    inline Tree<T> & parent() { return const_cast<Tree<T> &>(static_cast<const Tree<T> *>(this)->parent()); }
    inline Tree<T> & child(size_t index) { return const_cast<Tree<T> &>(static_cast<const Tree<T> *>(this)->child(index)); }

    std::vector<std::shared_ptr<Tree<T> > > getSubtrees(int root_level, int leaf_level) const {
        if (root_level < 0) throw std::runtime_error("Tree: you must specify root_level >= 0");
        if (leaf_level < 0) throw std::runtime_error("Tree: you must specify leaf_level >= 0");
        if (leaf_level < root_level) throw std::runtime_error("Tree: you must specify leaf_level >= root_level");

        std::vector<std::shared_ptr<Tree<T> > > roots;
        getSubtreesByNode(roots, root_level, leaf_level);
        return roots;
    }

    std::vector<T> getLeaves() const {
        std::vector<T> values;
        getLeavesByNode(values);
        return values;
    }

private:
    T value_;
    Tree<T> * parent_;
    std::vector<Tree<T> *> children_;

    void getSubtreesByNode(std::vector<std::shared_ptr<Tree<T> > > & roots, int root_level, int leaf_level) const {
        if (root_level > 0) {
            for (const Tree<T> * child : children_) {
                child->getSubtreesByNode(roots, root_level - 1, leaf_level - 1);
            }
        } else {
            roots.push_back(std::shared_ptr<Tree<T> >(copyTree(leaf_level - root_level)));
        }
    }

    Tree<T> * copyTree() const {
        Tree<T> * node = new Tree<T>(value_);
        for (const Tree<T> * child : children_) {
            node->addChild(child->copyTree());
        }
        return node;
    }

    Tree<T> * copyTree(int rest_depth) const {
        Tree<T> * node = new Tree<T>(value_);
        if (rest_depth > 0) {
            for (const Tree<T> * child : children_) {
                node->addChild(child->copyTree(rest_depth - 1));
            }
        }
        return node;
    }

    void getLeavesByNode(std::vector<T> & values) const {
        if (isLeaf()) {
            values.push_back(value_);
        } else {
            for (const Tree<T> * child : children_) {
                child->getLeavesByNode(values);
            }
        }
    }

}; // class Tree

} // namespace Ckylark

#endif // CKYLARK_TREE_H_

