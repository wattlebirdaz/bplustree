#include <vector>
#include <memory>

static const int MAX_THRESHOLD = 30;
static const int MIN_THRESHOLD = 5;

class node;

class record {
  friend bool operator<(const record& pLeft, const record& pRight);
public:
  record(int pKey);
  record(int pKey, int pValue);
  record(const record& pRecord);
  record& operator=(const record &pRecord);
  int get_key() const;
  int get_value() const;
private:
  int mKey;
  int mValue;
};

class link {
  friend class inner_node;
  friend class bplustree;
public:
  friend bool operator<(const link& pLeft, const link& pRight);
  link(int pKey);
  link(int pKey, bool pIsLeaf);
  link(const link &pLink);
  link& operator=(const link &pLink);
  void set_key(int pKey);
  int get_key() const;
  node* get_node() const;
private:
  int mKey;
  std::shared_ptr<node> mID;
};

class inner_node;

class node {
public:
  node(bool pIsLeaf);
  virtual ~node();
  virtual node* join_sibling_node() = 0;
  virtual int get_size() const = 0;
  virtual bool is_too_big() const = 0;
  virtual bool is_too_small() const = 0;
  virtual int get_key(int pIndex) const = 0;
  bool is_leaf() const;
  void set_parent(inner_node*);
  inner_node* get_parent() const;
  void set_prev(node*);
  node* get_prev() const;
  void set_next(node*);
  node* get_next() const;
private:
  bool mIsLeaf;
  inner_node *mParent;
  node *mPrev, *mNext;
};


class leaf_node: public node {
  friend class bplustree;
public:
  leaf_node();
  int get_size() const override;
  bool is_too_big() const override;
  bool is_too_small() const override;
  int get_key(int pIndex) const override;
  node* join_sibling_node() override;
  void add_record(const record& pRecord);
  void remove_record(int pKey);
  const record* search(int pKey) const;
private:
  std::vector<record> mRecords;
};

class inner_node: public node {
  friend class bplustree;
public:
  inner_node();
  inner_node(const std::shared_ptr<node> &pHeir);
  int get_size() const override;
  bool is_too_big() const override;
  bool is_too_small() const override;
  int get_key(int pIndex) const override;
  node* join_sibling_node() override;
  void add_link(const link &pLink);
  void remove_link(int pKey);
  node* search(int pKey) const;
private:
  std::shared_ptr<node> mHeir;
  std::vector<link> mLinks;
};

class bplustree {
public:
  bplustree();
  const record* search(int pKey) const;
  bool insert(const record& pRecord);
  bool remove(int pKey);
  void show_all() const;
private:
  std::shared_ptr<node> mRoot;
};
