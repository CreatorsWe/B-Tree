#include"B-Tree.h"
int main(){
  BTree<int> btree(4,5,7);
  btree.insert(4,5);
  btree.midorder();
  return 0;
}