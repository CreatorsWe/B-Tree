#ifndef _BTREE
#define _BTREE
#include <iostream>
#include<vector>
#include<memory>
#include<algorithm>
#include<utility>
#include<queue>
using namespace std;

template<typename T>
class Bnode;
/*B-树结点类型，动态扩容，使用vector存储 key 和 指针，第 i 个 key两边指针是 ptr[i] 和 ptr[i+1]
非叶子节点的指针必须指向一个有效地址，不会指向nullptr，叶子结点不分配指针空间，非叶子结点转化为叶
子结点要回收指针空间*/   
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
#include"B-Tree.tpp"

#endif


