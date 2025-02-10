#ifndef _BTREE
#define _BTREE
#include <iostream>
#include<vector>
#include<memory>
#include<algorithm>
#include<utility>
#include<queue>
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
  Bnode(const int& _index,const T& _data,const bool _isleaf = true); 
  Bnode(const pair<int,T>& _key,const bool _isleaf = true);
  Bnode(const vector<pair<int,T>>& _keyvec,const bool _isleaf = true);  //拷贝 key 容器
  Bnode(Bnode<T>* _node,const int& _start,const int& _end); //浅拷贝指定size大小的node空间
  Bnode(Bnode<T>&& _node);
  void printNode() const;
};
template<class T>
Bnode<T>::Bnode(const int& _index,const T& _data,const bool _isleaf){
  key.push_back(make_pair(_index,_data));
  if(_isleaf == true) ptr.resize(0);
  else ptr.insert(ptr.begin(),2,nullptr);
  isleaf = _isleaf;
};
template<class T>
Bnode<T>::Bnode(const pair<int,T>& _key,const bool _isleaf) {
  key.push_back(_key);
  if(_isleaf == true) ptr.resize(0);
  else ptr.insert(ptr.begin(),2,nullptr);
  isleaf = _isleaf;
};
template <class T>
inline Bnode<T>::Bnode(const vector<pair<int,T>>& _keyvec,const bool _isleaf):key(_keyvec){
  if(_isleaf == true) ptr.resize(0);    //叶子结点不分配指针空间
  else ptr.insert(ptr.begin(),_keyvec.size() + 1,nullptr); 
  isleaf = _isleaf;
}
template <class T>
inline Bnode<T>::Bnode(Bnode<T>* _node, const int& _start,const int& _end){
  if(_start < 0 || _end > _node->key.size() + 1) throw("index out of range");
  isleaf = _node->isleaf;
  std::copy(_node->key.begin() + _start,_node->key.begin() + _end,std::back_inserter(key));
  if(isleaf) ptr.resize(0);
  else  std::copy(_node->ptr.begin() + _start, _node->ptr.begin() + _end + 1, std::back_inserter(ptr));
};

template<class T>
Bnode<T>::Bnode(Bnode<T>&& _node){
  key(_node.key);
  ptr(_node.ptr);
  isleaf = _node.isleaf;
}

template <class T>
inline void Bnode<T>::printNode() const
{
  for(int i = 0; i < key.size();++i) cout<<key.at(i).first<<" ";
}

/*B-树：结点使用vector存储数据，实现动态扩容，使用智能指针管理内存，避免内存泄漏 */
template<class T>
class BTree{
private:
  int degree;  //度数，下限,一般是 ceil(order / 2)
  int order;   //阶数，上限
  shared_ptr<Bnode<T>> root;
  int size;
  Bnode<T>* copy(const shared_ptr<Bnode<T>>& _node);
  void preOrder(const shared_ptr<Bnode<T>>& _node) const;
  void midOrder(const shared_ptr<Bnode<T>>& _node) const;
  T* search(const shared_ptr<Bnode<T>>& _node,const int& _index);
  void Insert(shared_ptr<Bnode<T>>& _node,const int& _index,const T& _data,bool& _isupdate);
  void nodeHandle(shared_ptr<Bnode<T>>& _node, const int& _removepos,bool& _isupdate);
  int Remove(shared_ptr<Bnode<T>>& _node,const int& _index,bool& _isupdate);
  Bnode<T>* splitNode(shared_ptr<Bnode<T>>& _node,const int& _midpos,const bool _isfirstptrpos);
public: 
  BTree(const int& _order);
  BTree(const int& _order,const pair<int,T>& _key);
  BTree(const int& _order,const int& _index,const T& _data);
  BTree(const BTree<T>& _tree);
  BTree(BTree<T>&& _tree);
  bool search(const int& _index); //返回指向数据的指针
  void insert(const int& _index,const T& _data);
  void insert(const pair<int,T>& _key);
  bool remove(const int& _index);
  int height() const;    //所有叶子结点都在同一层，迭代即可
  void midorder() const; //中序遍历，递增序列
  void sequenceorder() const;
  bool empty() const;
  int length() const;
  int getorder() const;
  int getdegree() const;
};

template <class T>
Bnode<T>* BTree<T>::copy(const shared_ptr<Bnode<T>> &_node){
  if(_node->isleaf == true) return new Bnode<T>(_node->key,true);  //叶子结点，不分配指针空间
  Bnode<T>* tmp = new Bnode<T>(_node->key);   //非叶子节点，分配指针空间
  for(int i = 0;i<tmp->ptr.size();++i) tmp->ptr.at(i).reset(copy(_node->ptr.at(i)));
  return tmp;
}

template <class T>
inline void BTree<T>::midOrder(const shared_ptr<Bnode<T>> &_node) const
{
  if(_node->isleaf){
    _node->printNode();
    return;
  }
  for(int i = 0;i<_node->key.size();++i){
    midOrder(_node->ptr.at(i));
    cout<<_node->key.at(i).first<<" ";
  }
  midOrder(_node->ptr.at(_node->key.size()));
}

template <class T>
inline T* BTree<T>::search(const shared_ptr<Bnode<T>> &_node,const int& _index)
{
  if(_node->isleaf){
    int findpos = 0;
    for(;findpos < _node->key.size() && _index != _node->key.at(findpos).first;++findpos);
    if(findpos == _node->key.size()) return nullptr;
    else return &_node->key.at(findpos).second;
  }
  int findpos = 0;
  for(;findpos < _node->key.size() && _index > _node->key.at(findpos).first;++findpos);
  if(findpos < _node->key.size() && _index == _node->key.at(findpos).first) return &_node->key.at(findpos).second;
  return search(_node->ptr.at(findpos),_index);
}

template <class T>
inline void BTree<T>::Insert(shared_ptr<Bnode<T>> &_node,const int& _index,\
                                    const T& _data,bool& _isupdate)
{
  if(_node->isleaf){      //找到叶子结点
    int insertpos = 0;  //插入位置
    for(;insertpos < _node->key.size() && _index > _node->key.at(insertpos).first;++insertpos);
    _node->key.insert(_node->key.begin() + insertpos,make_pair(_index,_data));    //插入
    if(_node->key.size() <= order - 1)  _isupdate = false;  //未超出上限，不用更新
    ++size;
    return;
  }
  int findpos = 0;
  for(;findpos < _node->key.size() && _index > _node->key.at(findpos).first;++findpos);
  Insert(_node->ptr.at(findpos),_index,_data,_isupdate);
  if(_isupdate == false) return;
  //往上更新：
  //1.根据 finpos 找到返回结点
  //2.删除返回节点中间值，并将返回节点一分为二
  //3.将中间值插入该节点中，并增加指针指向新分裂的结点
  //4.判断增加中间值后是否超出上限，停止或继续更新
  shared_ptr<Bnode<T>>& insertnode = _node->ptr.at(findpos);  //返回结点
  int midpos = static_cast<int>((insertnode->key.size() / 2) - 1); //中间位置
  pair<int,T> midkey = insertnode->key.at(midpos);   
  bool isfirstptr = findpos == 0?true:false;
  Bnode<T>* newnodeptr = splitNode(insertnode,midpos,findpos == 0 ? true:false);
  _node->key.insert(_node->key.begin() + findpos,midkey);
  if(findpos == 0) _node->ptr.insert(_node->ptr.begin(),shared_ptr<Bnode<T>>(newnodeptr));
  else _node->ptr.insert(_node->ptr.begin() + findpos + 1,shared_ptr<Bnode<T>>(newnodeptr));
  if(_node->key.size() <= order -1) _isupdate = false;
}

template <class T>
inline void BTree<T>::nodeHandle(shared_ptr<Bnode<T>> &_node, const int& _removepos,bool &_isupdate)
{
  int _siblingtag = -1; //兄弟结点
  shared_ptr<Bnode<T>>& handlenode = _node->ptr.at(_removepos);
  bool isleaf = handlenode->isleaf;
  if(_removepos < _node->key.size() && _node->ptr.at(_removepos + 1)->key.size() >= degree) _siblingtag = 1;  //右兄弟可借
  else if(removepos > 0 && _node->ptr.at(_removepos - 1)->key.size() >= degree) _siblingtag = 0;  //左兄弟可借
  if(_siblingtag == 0){ //左兄弟可借
    shared_ptr<Bnode<T>>& leftsibling = _node->ptr.at(_removepos - 1);
    handlenode->key.push_front(_node->key.at(_removepos - 1));
    _node->key.at(_removepos - 1) = leftsibling->key.back();
    leftsibling->key.pop_back();
    if(!isleaf) handlenode->ptr.push_front(leftsibling->ptr.back());
    leftsibling->ptr.pop_back();
    _isupdate = false;
    return;
  }
  else if(_siblingtag == 1){  //右兄弟可借
    shared_ptr<Bnode<T>>& rightsibling = _node->ptr.at(_removepos + 1);
    handlenode->key.push_back(_node->key.at(_removepos));
    _node->key.at(_removepos) = rightsibling->key.front();
    rightsibling->key.pop_front();
    if(!isleaf) handlenode->ptr.push_back(rightsibling->ptr.front());
    rightsibling->ptr.pop_front();
    _isupdate = false;    
    return;
  }
  //没有兄弟结点可以借，合并兄弟节点
  if(_removepos == _node->key.size()){  //最右边的指针，只能合并左兄弟
    shared_ptr<Bnode<T>>& handlenode = _node->ptr.at(_removepos);
    shared_ptr<Bnode<T>>& leftsibling = _node->ptr.at(_removepos - 1);
    handlenode->key.push_front(_node->key.at(_removepos - 1));
    handlenode->key.insert(handlenode->key.begin(),leftsibling->key.begin(),leftsibling->key.end());
    if(!isleaf) handlenode->ptr.insert(handlenode->ptr.begin(),leftsibling->ptr.begin(),leftsibling->ptr.end());
    _node = handlenode;
    leftsibling = nullptr;
    _isupdate = false;
    return;
  }
  shared_ptr<Bnode<T>>& handlenode = _node->ptr.at(_removepos);
  shared_ptr<Bnode<T>>& rightsibling = _node->ptr.at(_removepos + 1);
  handlenode->key.push_back(_node->key.at(_removepos));
  handlenode->key.insert(handlenode->key.end(),rightsibling->key.begin(),rightsibling->key.end());
  if(!isleaf) handlenode->ptr.insert(handlenode->ptr.end(),rightsibling->ptr.begin(),rightsibling->ptr.end());
  _node = handlenode;
  rightsibling = nullptr;
  _isupdate = false;
  return;
}

template <class T>
inline int BTree<T>::Remove(shared_ptr<Bnode<T>> &_node, const int &_index, bool &_isupdate)
{
  if(_node->isleaf){  //叶子结点
    int removepos = 0;
    for(;removepos < _node->key.size() && _index != _node->key.at(removepos).first;++removepos);
    if(removepos == _node->key.size()) return -1; 
    else return removepos;
  }
  int removepos = 0;
  for(;removepos < _node->key.size() && _index < _node->key.at(removepos).first;++removepos);
  if(removepos < _node->key.size() && _index == _node->key.at(removepos).first) return removepos;
  int resultpos = Remove(_node->ptr.at(removepos),_index,_isupdate);
  if(resultpos == -1) return -1;  
  //找到关键字，删除关键字并处理失衡
}

template <class T>
inline Bnode<T> *BTree<T>::splitNode(shared_ptr<Bnode<T>> &_node, const int& _midpos,const bool _isfirstptrpos)
{
  if(_node->key.size() <= order - 1) throw("this node is not out of upper bound!");
  Bnode<T>* newBnode = nullptr;
  if(_isfirstptrpos){  //第一个指针位置，向前分裂
      newBnode = new Bnode<T>(_node.get(),0,_midpos);
      _node->key.erase(_node->key.begin(),_node->key.begin() + _midpos + 1);
      if(!_node->isleaf) _node->ptr.erase(_node->ptr.begin(),_node->ptr.begin() + _midpos + 1);
      return newBnode;
  }   //不是第一个指针位置，向后分裂
  newBnode = new Bnode<T>(_node.get(),_midpos + 1,_node->key.size());
  _node->key.erase(_node->key.begin() + _midpos,_node->key.end());
  if(!_node->isleaf) _node->ptr.erase(_node->ptr.begin() + _midpos + 1,_node->ptr.end());
  return newBnode;
};

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
inline bool BTree<T>::search(const int &_index)
{
  if(root == nullptr) return false;
  if(search(root,_index) == nullptr) return false;
  else return true;
}
template <class T>
inline void BTree<T>::insert(const int &_index, const T &_data)
{
  if(root == nullptr){
    root.reset(new Bnode<T>(_index,_data));
    ++size;
    return;
  }
  if(search(_index)) return;
  bool isupdate = true;
  Insert(root,_index,_data,isupdate);
  if(isupdate == false) return;
  //分裂根结点
  int midpos = static_cast<int>(root->key.size() / 2 - 1);
  pair<int,T> midkey = root->key.at(midpos);
  Bnode<T>* newsplit = splitNode(root,midpos,false);
  Bnode<T>* newroot = new Bnode<T>(midkey,false);
  newroot->ptr.at(0) = root;
  newroot->ptr.at(1).reset(newsplit);
  root.reset(newroot);
}
template <class T>
inline void BTree<T>::insert(const pair<int, T> &_key) {
  insert(_key.first,_key.second);
};

template <class T>
inline int BTree<T>::height() const
{
  if(root == nullptr) return 0;
  shared_ptr<Bnode<T>> tmp = root;
  int height = 1;
  while(!tmp->isleaf){
    ++height;
    tmp = tmp->ptr.at(0);
  }
  return height;
}

template <class T>
inline void BTree<T>::midorder() const
{
  if(root == nullptr) cout<<"null"<<endl;
  else midOrder(root);
  cout<<endl;
}

template <class T>
inline void BTree<T>::sequenceorder() const
{
  if(root == nullptr) cout<<"null"<<endl;
  queue<pair<shared_ptr<Bnode<T>>,int>> que; //int 存储层数
  que.push(make_pair(root,1));
  while(!que.empty()){
    auto tmp = que.front();
    que.pop();
    cout << tmp.second << " : ";
    tmp.first->printNode();
    cout << endl;
    if(!tmp.first->isleaf){
      for(int i = 0;i < tmp.first->ptr.size();++i) que.push(make_pair(tmp.first->ptr.at(i),tmp.second + 1));
    }
  }
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
}
template <class T>
inline int BTree<T>::getorder() const
{
  return order;
}
template <class T>
inline int BTree<T>::getdegree() const
{
  return degree;
};

#endif


