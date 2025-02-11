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
  pair<int,T> leftMax(shared_ptr<Bnode<T>>& _node,bool& _isupdate);
  bool Remove(shared_ptr<Bnode<T>>& _node,const int& _index,bool& _isupdate);
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
  if(_node->isleaf){      //找到叶子结点，在叶子结点插入
    int insertpos = 0;    //插入位置
    for(;insertpos < _node->key.size() && _index > _node->key.at(insertpos).first;++insertpos);
    _node->key.insert(_node->key.begin() + insertpos,make_pair(_index,_data));    //插入
    ++size;
    if(_node->key.size() <= order - 1)  _isupdate = false;   //未超出上限，不用更新,否则返回其父节点更新
    return;
  }
  int findpos = 0;   //递归向下查找的子树位置
  for(;findpos < _node->key.size() && _index > _node->key.at(findpos).first;++findpos);
  Insert(_node->ptr.at(findpos),_index,_data,_isupdate);
  if(_isupdate == false) return;   //返回到该结点未失衡,不用向上更新
  //否则,更新返回结点,即失衡点(大于上限)：
  //1.根据 finpos 找到返回结点就是失衡节点(因为出现失衡会返回其父节点处理)
  shared_ptr<Bnode<T>>& insertnode = _node->ptr.at(findpos);       //失衡结点
  int midpos = static_cast<int>((insertnode->key.size() / 2));     //失衡点的中间位置
  pair<int,T> midkey = insertnode->key.at(midpos);                 //失衡点中间关键字
  //2.从 midpos处分裂失衡点, 返回新分裂的结点。如果 findpos  == 0,向前分裂新节点,否则向后分裂                  
  Bnode<T>* newnodeptr = splitNode(insertnode,midpos,findpos == 0 ? true:false);
  //3.将中间值插入失衡点的父节点中，并增加指针指向新分裂的结点，findpos == 0只能增加左指针，否则增加右指针
  _node->key.insert(_node->key.begin() + findpos,midkey);
  if(findpos == 0) _node->ptr.insert(_node->ptr.begin(),shared_ptr<Bnode<T>>(newnodeptr));
  else _node->ptr.insert(_node->ptr.begin() + findpos + 1,shared_ptr<Bnode<T>>(newnodeptr));
  //4.判断增加中间值后是否超出上限，停止或继续更新
  if(_node->key.size() <= order -1) _isupdate = false;
}

template <class T>
inline void BTree<T>::nodeHandle(shared_ptr<Bnode<T>> &_node, const int& _removepos,bool &_isupdate)
{
  int _siblingtag = -1; //标志位，-1 表示没有兄弟可借，0 表示可借左兄弟 1 表示可借右兄弟
  shared_ptr<Bnode<T>>& handlenode = _node->ptr.at(_removepos); //失衡点
  bool isleaf = handlenode->isleaf;                  //判断是否是叶子结点
  if(_removepos < _node->key.size() && _node->ptr.at(_removepos + 1)->key.size() >= degree) _siblingtag = 1;  //右兄弟可借
  else if(_removepos > 0 && _node->ptr.at(_removepos - 1)->key.size() >= degree) _siblingtag = 0;              //左兄弟可借
  if(_siblingtag == 0){ //左兄弟可借
    shared_ptr<Bnode<T>>& leftsibling = _node->ptr.at(_removepos - 1);  //失衡点的左兄弟结点
    pair<int,T>& leftfather = _node->key.at(_removepos - 1);            //失衡点的左父亲关键字
    //1.将左父亲关键字 push 到 失衡点的最前面（保证有序）
    handlenode->key.insert(handlenode->key.begin(),leftfather); 
    //2.左兄弟最后面的关键字替换左父亲关键字
    leftfather = leftsibling->key.back();
    //3.删除左兄弟最后面的关键字
    leftsibling->key.pop_back();
    //4.如果不是叶子结点，改变左兄弟最后面的指针(因为左兄弟最后的关键字被删了)，将其 push 到失衡点最前面(失衡点在最前面增加了关键字)
    if(!isleaf) {
      handlenode->ptr.insert(handlenode->ptr.begin(),leftsibling->ptr.back());
      leftsibling->ptr.pop_back();
    }
    //5.兄弟可借，不会改变父节点关键字数量，因此不用向上检查
    _isupdate = false;
    return;
  }
  else if(_siblingtag == 1){  //右兄弟可借
    shared_ptr<Bnode<T>>& rightsibling = _node->ptr.at(_removepos + 1);  //右兄弟
    pair<int,T>& rightfather = _node->key.at(_removepos);                //右父亲关键字
    //1.将右父亲关键字 push 到失衡点最后面
    handlenode->key.push_back(rightfather);
    //2.用右兄弟最前面的关键字替换右父亲关键字
    rightfather = rightsibling->key.front();
    //3.删除右兄弟最前面的关键字
    rightsibling->key.erase(rightsibling->key.begin());
    //4.不是叶子结点,改变右兄弟最前面的指针,push 到失衡点的最后面
    if(!isleaf) {
      handlenode->ptr.push_back(rightsibling->ptr.front());
      rightsibling->ptr.erase(rightsibling->ptr.begin());
    }
    _isupdate = false;    
    return;
  }
  //没有兄弟结点可以借，合并兄弟节点
  if(_removepos == _node->key.size()){       //最右边的指针，只能合并左兄弟
    pair<int,T> leftfather = _node->key.at(_removepos - 1);               //失衡点的左父亲关键字 
    shared_ptr<Bnode<T>>& leftsibling = _node->ptr.at(_removepos - 1);    //失衡点的左兄弟结点
    //1.将左父亲关键字 push 到失衡点的最前面(保证有序)
    handlenode->key.insert(handlenode->key.begin(),leftfather);
    //2.将左兄弟全部 push 到失衡点的最前面
    handlenode->key.insert(handlenode->key.begin(),leftsibling->key.begin(),leftsibling->key.end());
    //3.如果不是叶子结点，左兄弟所有指针也需要复制
    if(!isleaf) handlenode->ptr.insert(handlenode->ptr.begin(),leftsibling->ptr.begin(),leftsibling->ptr.end());
    //4.删除左父亲关键字和左父亲的左指针(指向已合并的左兄弟)
    _node->key.erase(_node->key.begin() + _removepos - 1);
    _node->ptr.erase(_node->ptr.begin() + _removepos - 1);
    //5.检查父亲所在结点是否是根结点且为空(因为根结点允许只有一个关键字)，是则将根结点指向合并后的结点
    if(_node == root && _node->key.empty()) _node = handlenode;
    //6.合并操作需要借父亲关键字，导致父亲所在结点关键字减少，检查是否失衡
    else if(_node->key.size() >= degree - 1) _isupdate = false;
    return;
  }   //合并右兄弟
  pair<int,T> rightfather = _node->key.at(_removepos);                   //失衡点的右父亲关键字
  shared_ptr<Bnode<T>>& rightsibling = _node->ptr.at(_removepos + 1);    //失衡点的右兄弟结点
  //1.将右父亲关键字 push 到失衡点的最后面
  handlenode->key.push_back(_node->key.at(_removepos));
  //2.将右兄弟全部 push 到失衡点的最后面
  handlenode->key.insert(handlenode->key.end(),rightsibling->key.begin(),rightsibling->key.end());
  //3.如果不是叶子节点，右兄弟的指针也需要复制
  if(!isleaf) handlenode->ptr.insert(handlenode->ptr.end(),rightsibling->ptr.begin(),rightsibling->ptr.end());
  //4.删除右父亲关键字和右父亲的右指针
  _node->key.erase(_node->key.begin() + _removepos);
  _node->ptr.erase(_node->ptr.begin() + _removepos + 1);
  //5.检查父亲所在节点是否根节点且为空
  if(_node == root && _node->key.empty()) _node = handlenode;
  //6.检查父节点是否失衡
  else if(_node->key.size() >= degree - 1) _isupdate = false;
  return;
}

template <class T>
inline pair<int, T> BTree<T>::leftMax(shared_ptr<Bnode<T>> &_node, bool& _isupdate)
{
/*返回 并 删除 以_node为根节点的最大关键字(最右边),并保证路径平衡.*/
  if(_node->isleaf){  
    pair<int,T> maxkey = _node->key.back();
    _node->key.pop_back();
    --size;
    if(_node->key.size() >= degree - 1) _isupdate = false;
    return maxkey;
  }
  pair<int,T> maxkey = leftMax(_node->ptr.back(),_isupdate);
  if(_isupdate) nodeHandle(_node,_node->ptr.size() - 1,_isupdate);
  return maxkey;
}

template <class T>
inline bool BTree<T>::Remove(shared_ptr<Bnode<T>> &_node, const int &_index, bool &_isupdate)
{
  int removepos = 0;      //找到第一个不小于 _index 的关键字位置，等于则找到，否则在 沿位置指针查找
  for(;removepos < _node->key.size() && _index > _node->key.at(removepos).first;++removepos);
  if(_node->isleaf && (removepos == _node->key.size() || _index != \
                  _node->key.at(removepos).first)) {_isupdate = false;return false;} //未找到
  //1.该结点找到关键字
  if(removepos < _node->key.size() && _index == _node->key.at(removepos).first){  
    //case1.1：删除关键字所在结点是叶子结点,先删除
    if(_node->isleaf){
      _node->key.erase(_node->key.begin() + removepos);
      --size;
      //删除后不小于下限，不需要更新(_isupate == false),否则返回父节点更新
      if(_node->key.size() >= degree - 1) _isupdate = false;
      return true;  
    }
    //case1.2:不在叶子结点,找到左子树最大值代替,由于B-Tree每个指针都不会为 nullptr,所以左子树必存在
    _node->key.at(removepos) = leftMax(_node->ptr.at(removepos),_isupdate); //leftMax返回最大值,并保证查找路径上的平衡
    //_node->ptr.at(_removepos)如果出现失衡,返回其父节点 _node 处理
    if(_isupdate) nodeHandle(_node,removepos,_isupdate); //nodeHandle函数处理失衡
    return true;
  }
  //2.该结点未找到关键字,沿 removepos 的指针递归查找
  bool result = Remove(_node->ptr.at(removepos),_index,_isupdate);
  //3.检查并处理返回的子节点是否失衡
  if(_isupdate) nodeHandle(_node,removepos,_isupdate);
  return result;
}

template <class T>
inline Bnode<T> *BTree<T>::splitNode(shared_ptr<Bnode<T>> &_node, const int& _midpos,const bool _isfirstptrpos)
{
/*作用:先将 _node从_midpos处分裂出一个新节点,返回新节点指针,_isfirstptrpos 第一个指针指向的结点只能向前分裂,否则向后分裂*/
  //1.如果_node关键字数量不大于上限,则不需需要分裂,抛错
  if(_node->key.size() <= order - 1) throw("this node is not out of upper bound!");
  Bnode<T>* newBnode = nullptr;  //新分裂结点的指针
  if(_isfirstptrpos){     //第一个指针位置，向前分裂
      //调用 Bnode 的范围拷贝函数,它会浅拷贝 [start,end) 的关键字和 [start,end] 的指针(如果不是叶子结点)
      newBnode = new Bnode<T>(_node.get(),0,_midpos);
      //将原节点 分裂出的部分 +  _midpos位置 的关键字删除
      _node->key.erase(_node->key.begin(),_node->key.begin() + _midpos + 1);
      //不是叶子节点,删除 分裂部分 + _midpos 的左指针
      if(!_node->isleaf) _node->ptr.erase(_node->ptr.begin(),_node->ptr.begin() + _midpos + 1);
      return newBnode;
  }                   //不是第一个指针位置，向后分裂
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
  if(isupdate == false) return;  //Insert函数不会处理根结点失衡
  //分裂根结点
  int midpos = static_cast<int>(root->key.size() / 2);
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
}
template <class T>
inline bool BTree<T>::remove(const int &_index)
{
  if(root == nullptr) return false;
  bool isupdate = true;
  return Remove(root,_index,isupdate); //Remove会处理根结点失衡(根结点为空).
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


