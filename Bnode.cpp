#include<iostream>
#include<vector>
#include<utility>
#include<memory>
using namespace std;
/*B树结点类*/
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
Bnode<T>::Bnode(const vector<pair<int,T>>& _keyvec,const bool _isleaf):key(_keyvec){
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