#include "bplustree.hpp"

#include <algorithm>
#include <iostream>

// comparator /////////////////////////////////////////////////////////////////

bool operator<(const record &pLeft, const record &pRight) {
  if (pLeft.mKey < pRight.mKey) {
    return true;
  } else {
    return false;
  }
}

bool operator<(const link &pLeft, const link &pRight) {
  if (pLeft.mKey < pRight.mKey) {
    return true;
  } else {
    return false;
  }
}

// record class ///////////////////////////////////////////////////////////////

record::record(int pKey) : mKey(pKey) {}

record::record(int pKey, int pValue) : mKey(pKey), mValue(pValue) {}

record::record(const record &pRecord)
    : mKey(pRecord.mKey), mValue(pRecord.mValue) {}

record &record::operator=(const record &pRecord) {
  if (&pRecord != this) {
    mKey = pRecord.mKey;
    mValue = pRecord.mValue;
  }
  return *this;
}

int record::get_key() const { return mKey; }

int record::get_value() const { return mValue; }

// link class /////////////////////////////////////////////////////////////////

link::link(int pKey) : mKey(pKey) {}

link::link(int pKey, bool pIsLeaf) : mKey(pKey) {
  if (pIsLeaf) {
    mID = std::make_shared<leaf_node>();
  } else {
    mID = std::make_shared<inner_node>();
  }
}

link::link(const link &pLink) : mKey(pLink.mKey), mID(pLink.mID) {}

link &link::operator=(const link &pLink) {
  if (&pLink != this) {
    mKey = pLink.mKey;
    mID = pLink.mID;
  }
  return *this;
}

void link::set_key(int pKey) { mKey = pKey; }

int link::get_key() const { return mKey; }

node *link::get_node() const { return mID.get(); }

// node class /////////////////////////////////////////////////////////////////
node::node(bool pIsLeaf)
    : mIsLeaf(pIsLeaf), mParent(nullptr), mPrev(nullptr), mNext(nullptr) {}

node::~node() {}

bool node::is_leaf() const { return mIsLeaf; }

void node::set_parent(inner_node *pParent) { mParent = pParent; }

inner_node *node::get_parent() const { return mParent; }

void node::set_prev(node *pPrev) { mPrev = pPrev; }

node *node::get_prev() const { return mPrev; }

void node::set_next(node *pNext) { mNext = pNext; }

node *node::get_next() const { return mNext; }

// leaf_node class ////////////////////////////////////////////////////////////
leaf_node::leaf_node() : node(true) {}

int leaf_node::get_size() const { return mRecords.size(); }

bool leaf_node::is_too_big() const { return mRecords.size() > MAX_THRESHOLD; }

bool leaf_node::is_too_small() const { return mRecords.size() < MIN_THRESHOLD; }

int leaf_node::get_key(int pIndex) const { return mRecords[pIndex].get_key(); }

void leaf_node::add_record(const record &pRecord) {
  auto it = std::upper_bound(mRecords.begin(), mRecords.end(), pRecord);
  mRecords.insert(it, pRecord);
}

void leaf_node::remove_record(int pKey) {
  record r(pKey);
  auto it1 = std::lower_bound(mRecords.begin(), mRecords.end(), r);
  auto it2 = std::upper_bound(mRecords.begin(), mRecords.end(), r);
  mRecords.erase(it1, it2);
}

const record *leaf_node::search(int pKey) const {
  record r(pKey);
  auto it1 = std::lower_bound(mRecords.begin(), mRecords.end(), r);
  auto it2 = std::upper_bound(mRecords.begin(), mRecords.end(), r);
  if (it2 - it1 == 0) {
    return nullptr;
  } else {
    return &(*it1);
  }
}

node *leaf_node::join_sibling_node() {
  leaf_node *merge_subject;
  leaf_node *prev = static_cast<leaf_node *>(get_prev());
  leaf_node *next = static_cast<leaf_node *>(get_next());
  inner_node *parent = get_parent();
  int size = mRecords.size();
  if (prev && prev->get_parent() == parent &&
      prev->get_size() + size <= MAX_THRESHOLD) {
    // merge with prev
    merge_subject = prev;
    if (get_next()) {
      get_next()->set_prev(merge_subject);
    }
    merge_subject->set_next(get_next());
  } else if (next && next->get_parent() == parent &&
             next->get_size() + size <= MAX_THRESHOLD) {
    // merge wtih next
    merge_subject = next;
    if (get_prev()) {
      get_prev()->set_next(merge_subject);
    }
    merge_subject->set_prev(get_prev());
  } else {
    merge_subject = nullptr;
  }
  if (merge_subject) {
    for (const auto &r : mRecords) {
      merge_subject->add_record(r);
    }
  }
  return merge_subject;
}

// inner_node class ///////////////////////////////////////////////////////////
inner_node::inner_node() : node(false), mHeir(nullptr) {}

inner_node::inner_node(const std::shared_ptr<node> &pHeir)
    : node(false), mHeir(pHeir) {}

int inner_node::get_size() const { return mLinks.size(); }

bool inner_node::is_too_big() const { return mLinks.size() > MAX_THRESHOLD; }

bool inner_node::is_too_small() const { return mLinks.size() < MIN_THRESHOLD; }

int inner_node::get_key(int pIndex) const { return mLinks[pIndex].get_key(); }

node *inner_node::join_sibling_node() {
  inner_node *merge_subject;
  inner_node *prev = static_cast<inner_node *>(get_prev());
  inner_node *next = static_cast<inner_node *>(get_next());
  inner_node *parent = get_parent();
  int size = mLinks.size();
  if (!parent) {
    return nullptr;
  }
  if (prev && prev->get_parent() == parent &&
      prev->get_size() + size <= MAX_THRESHOLD) {
    // merge with prev
    merge_subject = prev;
    if (get_next()) {
      get_next()->set_prev(merge_subject);
    }
    merge_subject->set_next(get_next());

    // move heir to merge subject
    for (auto it = parent->mLinks.begin(); it != parent->mLinks.end(); it++) {
      if (it->get_node() == static_cast<node *>(this)) {
        link l(it->get_key());
        l.mID = mHeir;
        merge_subject->add_link(l);
        break;
      }
    }

    // move other links to merge subject
    for (const auto &l : mLinks) {
      merge_subject->add_link(l);
    }

  } else if (next && next->get_parent() == parent &&
             next->get_size() + size <= MAX_THRESHOLD) {
    // merge wtih next
    merge_subject = next;
    if (get_prev()) {
      get_prev()->set_next(merge_subject);
    }
    merge_subject->set_prev(get_prev());

    // change heir of merge subject to link
    for (auto it = parent->mLinks.begin(); it != parent->mLinks.end(); it++) {
      if (it->get_node() == static_cast<node *>(merge_subject)) {
        link l(it->get_key());
        l.mID = merge_subject->mHeir;
        merge_subject->add_link(l);
        break;
      }
    }

    // move heir and links to merge subject
    merge_subject->mHeir = mHeir;
    for (const auto &l : mLinks) {
      merge_subject->add_link(l);
    }

  } else {
    merge_subject = nullptr;
  }
  return merge_subject;
}

void inner_node::add_link(const link &pLink) {
  auto it = std::upper_bound(mLinks.begin(), mLinks.end(), pLink);
  mLinks.insert(it, pLink);
}

void inner_node::remove_link(int pKey) {
  link l(pKey);
  auto it1 = std::lower_bound(mLinks.begin(), mLinks.end(), l);
  auto it2 = std::upper_bound(mLinks.begin(), mLinks.end(), l);
  mLinks.erase(it1, it2);
}

node *inner_node::search(int pKey) const {
  link l(pKey);
  auto it = std::upper_bound(mLinks.begin(), mLinks.end(), l);
  if (it == mLinks.begin()) {
    return mHeir.get();
  } else {
    return (--it)->get_node();
  }
}

// bplustree class ////////////////////////////////////////////////////////////
bplustree::bplustree() { mRoot = std::make_shared<leaf_node>(); }

const record *bplustree::search(int pKey) const {
  if (!mRoot) {
    return nullptr;
  }

  node *temp = mRoot.get();
  while (!temp->is_leaf()) {
    inner_node *itemp = dynamic_cast<inner_node *>(temp);
    if (itemp) {
      temp = itemp->search(pKey);
    } else {
      return nullptr;
    }
  }

  leaf_node *ltemp = dynamic_cast<leaf_node *>(temp);
  if (ltemp) {
    return ltemp->search(pKey);
  } else {
    return nullptr;
  }
}

bool bplustree::insert(const record &pRecord) {
  if (!mRoot) {
    return false;
  }
  node *temp = mRoot.get();
  inner_node *itemp;
  leaf_node *ltemp;

  // add record into the leaf
  while (!temp->is_leaf()) {
    itemp = dynamic_cast<inner_node *>(temp);
    if (itemp) {
      temp = itemp->search(pRecord.get_key());
    } else {
      return false;
    }
  }
  ltemp = dynamic_cast<leaf_node *>(temp);
  ltemp->add_record(pRecord);

  // split if needed
  // split the original node and create&fill the new node
  while (true) {
    // 0. find out the current node type
    bool isLeaf = temp->is_leaf();

    // 1. get size of the leaf_node
    // 2. get the first key of the second half to create a link
    if (isLeaf) {
      // if leaf
      leaf_node *lOriginal = static_cast<leaf_node *>(temp);
      if (!lOriginal->is_too_big())
        break; // break if no split is needed
      int size = lOriginal->get_size();
      int linkKey = lOriginal->get_key(size / 2);
      inner_node *iParent = lOriginal->get_parent();

      link l(linkKey, isLeaf);
      // 3. set up new node in the link
      node *n = l.get_node();
      n->set_prev(lOriginal);
      n->set_next(lOriginal->get_next());
      lOriginal->set_next(n);

      leaf_node *lNew = dynamic_cast<leaf_node *>(n);
      auto halfIter = std::next(lOriginal->mRecords.begin(), size / 2);
      auto endIter = lOriginal->mRecords.end();
      lNew->mRecords.insert(lNew->mRecords.begin(), halfIter, endIter);
      // std::cout << "Leaf Original: ";
      // for (const auto& x: lOriginal->mRecords) {
      //   std::cout << x.get_key() << " ";
      // }
      // std::cout << std::endl;
      lOriginal->mRecords.erase(halfIter, endIter);
      // std::cout << "Leaf New: ";
      // for (const auto& x: lNew->mRecords) {
      //   std::cout << x.get_key() << " ";
      // }
      // std::cout << std::endl;

      if (iParent) {
        // if not root
        // set the parent of the new node
        n->set_parent(iParent);
        iParent->add_link(l);
        // loop
        temp = iParent;
      } else {
        // if root
        // a. create another innernode and set it as root
        mRoot = std::make_shared<inner_node>(mRoot);
        // b. set link
        dynamic_cast<inner_node *>(mRoot.get())->add_link(l);
        // c. set it as parent of original&new node
        temp->set_parent(static_cast<inner_node *>(mRoot.get()));
        n->set_parent(static_cast<inner_node *>(mRoot.get()));
        // end loop
        break;
      }
    } else {
      // if inner node
      inner_node *iOriginal = static_cast<inner_node *>(temp);
      if (!iOriginal->is_too_big())
        break; // break if no split is needed
      int size = iOriginal->get_size();
      int heirKey = iOriginal->get_key(size / 2);
      // int linkKey = iOriginal->get_key(size/2+1); // size/2+1 because of heir
      inner_node *iParent = iOriginal->get_parent();

      link l(heirKey, isLeaf);
      // 3. setup new node in the link
      node *n = l.get_node();
      n->set_prev(iOriginal);
      n->set_next(iOriginal->get_next());
      iOriginal->set_next(n);

      inner_node *iNew = dynamic_cast<inner_node *>(n);
      auto halfIter = std::next(iOriginal->mLinks.begin(), size / 2);
      auto endIter = iOriginal->mLinks.end();
      // change the parent node of child to the new node
      for (auto it = halfIter; it != endIter; it++) {
        it->get_node()->set_parent(iNew);
      }
      iNew->mHeir = halfIter->mID;
      iNew->mLinks.insert(iNew->mLinks.begin(), std::next(halfIter, 1),
                          endIter);

      // std::cout << "Inner Original: ";
      // for (const auto &x: iOriginal->mLinks) {
      //   std::cout << x.get_key() << " ";
      // }
      // std::cout << std::endl;
      iOriginal->mLinks.erase(halfIter, endIter);
      // std::cout << "Inner New: ";
      // for (const auto &x: iNew->mLinks) {
      //   std::cout << x.get_key() << " ";
      // }
      // std::cout << std::endl;

      if (iParent) {
        // if not root
        // set the parent of the new node
        n->set_parent(iParent);
        iParent->add_link(l);
        // loop
        temp = iParent;
      } else {
        // if root
        // a. create another innernode and set it as root
        mRoot = std::make_shared<inner_node>(mRoot);
        // b. set link
        dynamic_cast<inner_node *>(mRoot.get())->add_link(l);
        // c. set it as parent of original&new node
        temp->set_parent(static_cast<inner_node *>(mRoot.get()));
        n->set_parent(static_cast<inner_node *>(mRoot.get()));
        // end loop
        break;
      }
    }
  }
  return true;
}

bool bplustree::remove(int pKey) {
  if (!mRoot) {
    return false;
  }
  node *temp = mRoot.get();
  inner_node *itemp;
  leaf_node *ltemp;

  // if key found in leaf, remove the record
  while (!temp->is_leaf()) {
    itemp = dynamic_cast<inner_node *>(temp);
    if (itemp) {
      temp = itemp->search(pKey);
    } else {
      return false;
    }
  }
  ltemp = dynamic_cast<leaf_node *>(temp);
  if (!ltemp->search(pKey)) {
    // key not found in leaf
    return false;
  }
  ltemp->remove_record(pKey);

  bool hasSmallerKey = false;
  int newSmallestKey;
  if (ltemp->get_size() > 0 && pKey < ltemp->get_key(0)) {
    newSmallestKey = ltemp->get_key(0);
    hasSmallerKey = true;
  } else if (ltemp->get_size() == 0 && ltemp->get_next() &&
             static_cast<leaf_node *>(ltemp->get_next())->get_size() > 0) {
    newSmallestKey = static_cast<leaf_node *>(ltemp->get_next())->get_key(0);
    hasSmallerKey = true;
  }
  if (hasSmallerKey) {
    // std::cout << "newSmallestKey = " << newSmallestKey << std::endl;
    node *newTemp = ltemp;
    inner_node *ip = ltemp->get_parent();
    while (true) {
      if (ip && ip->mHeir.get() == newTemp) {
        newTemp = ip;
        ip = ip->get_parent();
      } else {
        // std::cout << ip->get_key(0) << std::endl;
        break;
      }
    }
    if (ip) {
      for (auto it = ip->mLinks.begin(); it != ip->mLinks.end(); it++) {
        if (it->get_node() == newTemp) {
          // std::cout << pKey << " should be the same as " << it->get_key() <<
          // std::endl;
          it->set_key(newSmallestKey);
        }
      }
    }
  }

  // merge if needed
  while (true) {
    // 0. find out the current node type
    bool isLeaf = temp->is_leaf();
    // 1. get size of the leaf_nodea
    // 2. get the first key of the second half to create a link
    if (isLeaf) {
      leaf_node *lWillBeDeleted = static_cast<leaf_node *>(temp);
      if (!lWillBeDeleted->is_too_small())
        break;
      inner_node *iParent = lWillBeDeleted->get_parent();

      if (iParent) {
        lWillBeDeleted = static_cast<leaf_node *>(temp);
        // std::cout << "lWillBeDeleted: ";
        // for (const auto x: lWillBeDeleted->mRecords) {
        //   std::cout << x.get_key() << " ";
        // }
        // std::cout << std::endl;
        leaf_node *lNew =
            static_cast<leaf_node *>(lWillBeDeleted->join_sibling_node());
        if (lNew) {
          // joined
          // std::cout << "leaf joined, lNew: ";
          // for (const auto x: lNew->mRecords) {
          //   std::cout << x.get_key() << " ";
          // }
          // std::cout << std::endl;

          // adujst the parent link to the WillBeDeletedNode and the NewNode
          if (iParent->mHeir.get() == lWillBeDeleted) {
            // if heir points to the WillBeDeletedNode
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == lNew) {
                iParent->mHeir = it->mID;
                iParent->mLinks.erase(it);
                break;
              }
            }
          } else if (iParent->mHeir.get() == lNew) {
            // if heir points to the NewNode
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == lWillBeDeleted) {
                iParent->mLinks.erase(it);
                break;
              }
            }
          } else {
            // if links point to the WillBeDeletedNode and the NewNode
            std::vector<link>::iterator it1, it2;
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == lWillBeDeleted) {
                it1 = it;
              }
              if (it->get_node() == lNew) {
                it2 = it;
              }
            }
            if (it1->get_key() < it2->get_key()) {
              it2->set_key(it1->get_key());
            }
            iParent->mLinks.erase(it1);
          }
          // loop
          temp = iParent;

        } else {
          // std::cout << "leaf did not join, not deleted" << std::endl;
          // did not join, end loop
          break;
        }
      } else {
        // root
        // if root is a leaf, end loop
        break;
      }
    } else {
      // if inner node
      inner_node *iWillBeDeleted = static_cast<inner_node *>(temp);
      if (!iWillBeDeleted->is_too_small())
        break;
      inner_node *iParent = iWillBeDeleted->get_parent();

      if (iParent) {
        iWillBeDeleted = static_cast<inner_node *>(temp);
        inner_node *iNew =
            static_cast<inner_node *>(iWillBeDeleted->join_sibling_node());

        // std::cout << "iWillBeDeleted: ";
        // for (const auto x: iWillBeDeleted->mLinks) {
        //   std::cout << x.get_key() << " ";
        // }
        // std::cout << std::endl;

        if (iNew) {
          // std::cout << "inner joined, iNew: ";
          // for (const auto x: iNew->mLinks) {
          //   std::cout << x.get_key() << " ";
          // }
          // std::cout << std::endl;

          // joined
          // set the parent of the child of WillBeDeletedNode to NewNode
          iWillBeDeleted->mHeir->set_parent(iNew);
          for (auto it = iWillBeDeleted->mLinks.begin();
               it != iWillBeDeleted->mLinks.end(); it++) {
            it->get_node()->set_parent(iNew);
          }

          // adujst the parent link to the WillBeDeletedNode and the NewNode
          if (iParent->mHeir.get() == iWillBeDeleted) {
            // if heir points to the WillBeDeletedNode
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == iNew) {
                iParent->mHeir = it->mID;
                it = iParent->mLinks.erase(it);
                break;
              }
            }
          } else if (iParent->mHeir.get() == iNew) {
            // if heir points to the NewNode
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == iWillBeDeleted) {
                it = iParent->mLinks.erase(it);
                break;
              }
            }
          } else {
            // if links point to the WillBeDeletedNode and the NewNode
            std::vector<link>::iterator it1, it2;
            for (auto it = iParent->mLinks.begin(); it != iParent->mLinks.end();
                 it++) {
              if (it->get_node() == iWillBeDeleted) {
                it1 = it;
              }
              if (it->get_node() == iNew) {
                it2 = it;
              }
            }
            if (it1->get_key() < it2->get_key()) {
              it2->set_key(it1->get_key());
            }
            iParent->mLinks.erase(it1);
          }
          // loop
          temp = iParent;
        } else {
          // std::cout << "inner did not join, not deleted" << std::endl;
          // did not join, end loop
          break;
        }
      } else {
        // if root is a inner node
        // if threre is no link, make heir as root
        if (static_cast<inner_node *>(mRoot.get())->get_size() == 0) {
          // std::cout << "Root node deleted" << std::endl;
          mRoot = static_cast<inner_node *>(mRoot.get())->mHeir;
        }
        // end loop
        break;
      }
    }
  }
  return true;
}

void bplustree::show_all() const {
  if (!mRoot) {
    return;
  }
  node *temp = mRoot.get();
  inner_node *itemp;
  leaf_node *ltemp;

  // if key found in leaf, remove the record
  while (!temp->is_leaf()) {
    itemp = dynamic_cast<inner_node *>(temp);
    if (itemp) {
      temp = itemp->mHeir.get();
    } else {
      return;
    }
  }
  ltemp = dynamic_cast<leaf_node *>(temp);
  while (true) {
    for (const auto r : ltemp->mRecords) {
      std::cout << "[key: " << r.get_key() << ", value: " << r.get_value()
                << "]";
    }
    std::cout << std::endl;
    if (ltemp->get_next()) {
      ltemp = static_cast<leaf_node *>(ltemp->get_next());
    } else {
      break;
    }
  }
}

int main() {
  bplustree bt;

  int n = 1000;

  for (int i = 0; i < n; i++) {
    record r(i, i);
    bt.insert(r);
  }

  for (int i = n - 1; i >= 0; i--) {
    if (i % 5 == 0) {
      std::cout << "deleting: " << i << std::endl;
      bt.remove(i);
    }
  }

  for (int i = 0; i < n; i++) {
    if (!bt.search(i)) {
      std::cout << "key: " << i << " deleted" << std::endl;
    }
  }
}
