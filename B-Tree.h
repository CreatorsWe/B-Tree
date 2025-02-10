#ifndef _BTREE
#define _BTREE
#include <iostream>
#include<vector>
#include<memory>
#include<utility>
using namespace std;

/*B-树结点类型，动态扩容，使用vector存储 key 和 指针，第 i 个 key两边指针是 ptr[i] 和 ptr[i+1]
非叶子节点的指针必须指向一个有效地址，不会指向nullptr，叶子结点不分配指针空间，非叶子结点转化为叶
子结点要回收指针空间*/
template<class T> //数据类型为 T
class Bnode{
public:
  vector<pair<int,T>> key;   
  vector<shared_ptr<Bnode<T>>> ptr;
  bool isleaf;   //叶子结点不分配指针，即ptr.reset(0),结点默认为 叶子结点
public:
  Bnode():key(),ptr(),isleaf(true){}; //会调用vector的无参构造函数
  Bnode(const int& _index,const T& _data); 
  Bnode(const pair<int,T>& _key);
  Bnode(const vector<pair<int,T>>& _keyvec,const bool _isleaf = false);  //拷贝 key 容器
  Bnode(Bnode<T>&& _node);
  void leaf(); //leaf函数，回收指针空间
};
template<class T>
Bnode<T>::Bnode(const int& _index,const T& _data){
  key.push_back(make_pair(_index,_data));
  ptr.resize(0);
  isleaf = true;
};
template<class T>
Bnode<T>::Bnode(const pair<int,T>& _key) {
  key.push_back(_key);
  ptr.resize(0);
  isleaf = true;
};
template <class T>
inline Bnode<T>::Bnode(const vector<pair<int,T>>& _keyvec,const bool _isleaf):key(_keyvec){
  if(_isleaf == true) ptr.resize(0);    //叶子结点不分配指针空间
  else ptr.insert(ptr.begin(),_keyvec.size() + 1,nullptr); 
  isleaf = _isleaf;
};
template<class T>
Bnode<T>::Bnode(Bnode<T>&& _node){
  key(move(_node.key));
  ptr(move(_node.ptr));
  isleaf = _node.isleaf;
}
template <class T>
inline void Bnode<T>::leaf() {
  ptr.resize(0);
  isleaf = true;
};

/*B-树：结点使用vector存储数据，实现动态扩容，使用智能指针管理内存，避免内存泄漏 */
template<class T>
class BTree{
private:
  int degree;  //度数，下限,一般是 ceil(order / 2)
  int order;   //阶数，上限
  shared_ptr<Bnode<T>> root;
  int size;
  Bnode<T>* copy(const shared_ptr<Bnode<T>>& _node);
  void printNode(const shared_ptr<Bnode<T>>& _node) const;
  void preOrder(const shared_ptr<Bnode<T>>& _node) const;
  void midOrder(const shared_ptr<Bnode<T>>& _node) const;
  pair<int,T> Insert(shared_ptr<Bnode<T>>& _node,const int& _index,const T& _data,bool& _isupdate);
public: 
  BTree(const int& _order);
  BTree(const int& _order,const pair<int,T>& _key);
  BTree(const int& _order,const int& _index,const T& _data);
  BTree(const BTree<T>& _tree);
  BTree(BTree<T>&& _tree);
  void insert(const int& _index,const T& _data);
  void insert(const pair<int,T>& _key);
  bool remove(const int& _index);
  int height() const;    //所有叶子结点都在同一层，迭代即可
  void preorder() const; //递增序列
  void midorder() const; //先打印根结点所有索引
  bool empty() const;
  int length() const;
};

template <class T>
Bnode<T>* BTree<T>::copy(const shared_ptr<Bnode<T>> &_node){
  if(_node->isleaf == true) return new Bnode<T>(_node->key,true);  //叶子结点，不分配指针空间
  Bnode<T>* tmp = new Bnode<T>(_node->key);   //非叶子节点，分配指针空间
  for(int i = 0;i<tmp->ptr.size();++i) tmp->ptr.at(i).reset(copy(_node->ptr.at(i)));
  return tmp;
}

template <class T>
inline void BTree<T>::printNode(const shared_ptr<Bnode<T>> &_node) const
{
  for(int i = 0; i < _node->key.size();++i) cout<<_node->key.at(i).frist<<" ";

} 

template <class T>
inline void BTree<T>::preOrder(const shared_ptr<Bnode<T>> &_node) const {  //打印结点 key
  if(_node->isleaf){
    printNode(_node);
    return;
  }
  for(int i = 0;i<_node->key.size();++i){
    cout<<_node->key.at(i).first<<" ";
    preOrder(_node->ptr.at(i));
  }
  preOrder(_node->ptr.at(_node->key.size()));
}

template <class T>
inline void BTree<T>::midOrder(const shared_ptr<Bnode<T>> &_node) const
{
  if(_node->isleaf){
    printNode(_node);
    return;
  }
  printNode(_node);
  for(int i = 0;i <= _node->key.size();++i) midOrder(ptr.at(i));
}

template <class T>
inline pair<int,T> BTree<T>::Insert(shared_ptr<Bnode<T>> &_node,const int& _index,const T& _data,bool& _isupdate)
{
  if(_node->isleaf){      //找到叶子结点
    if(_node->key.empty()) {_node.push_back(make_pair(_index,_data));}  //叶子结点为空,插入
    else{
      int insertpos = 0;  //插入位置
      for(;insertpos<_node->key.size();++insertpos){
        if(_data < _node->key.at(insertpos)) break;
      }
      _node->key.insert(_node->key.begin() + insertpos,make_pair(_index,_data));    //插入
    }
    if(_node->key.size() <= order - 1) { _isupdate = false;return;}

  }


}

template <class T>
BTree<T>::BTree(const int &_order)
{
  order = _order;
  degree = static_cast<int>((_order + 1) / 2);
  root = nullptr;
  size = 0;
}

template <class T>
inline BTree<T>::BTree(const int &_order, const pair<int, T> &_key){
  order = _order;
  degree = static_cast<int>((order + 1) / 2);
  root.reset(new Bnode<T>(_key));
  size = 1;
}

template <class T>
inline BTree<T>::BTree(const int &_order, const int &_index, const T &_data)
{
  order = _order;
  degree = static_cast<int>((order + 1) / 2);
  root.reset(new Bnode<T>(_index,_data));
  size = 1;
}

template<class T>
BTree<T>::BTree(const BTree<T>& _tree){
  order = _tree.order;
  degree = _tree.degree;
  size  = _tree.size;
  if(_tree.root == nullptr) root = nullptr;
  else root.reset(copy(_tree.root));
}

template <class T>
inline BTree<T>::BTree(BTree<T> &&_tree){
  degree = _tree.degree;
  order = _tree.order;
  size = _tree.size;
  root = move(_tree.root);
}
template <class T>
inline void BTree<T>::insert(const int &_index, const T &_data) {
  if(root == nullptr){
    root.reset(new Bnode<T>(_index,_data));
    ++size;
    return;
  }
  Insert(root);
};

template <class T>
inline int BTree<T>::height() const
{
  if(root == nullptr) return 0;
  shared_ptr<Bnode>& tmp = root;
  int height = 1;
  while(!tmp->isleaf){
    ++height;
    tmp = tmp->ptr.at(0);
  }
  return height;
}

template <class T>
inline void BTree<T>::preorder() const
{
  if(root == nullptr) cout<<"null"<<endl;
  else preOrder(root);
}

template <class T>
inline void BTree<T>::midorder() const
{
  if(root == nullptr) cout<<"null"<<endl;
  else midOrder(root);
}

template <class T>
inline bool BTree<T>::empty() const
{
  if(root == nullptr || size == 0) return false;
  return true;
}

template <class T>
inline int BTree<T>::length() const
{
  return size;
};


#endif
