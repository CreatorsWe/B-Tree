#ifndef _BNODE
#define _BNODE
#include<iostream>
#include<vector>
#include<utility>
#include<memory>
using namespace std;
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
#include"Bnode.tpp"

#endif