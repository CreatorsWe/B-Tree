// B-Tree.cpp: 定义应用程序的入口点。
//

#include "B-Tree.h"


int main()
{
	BTree<int> btree(4,make_pair(3,5));
	BTree<int> btree1(btree);
	return 0;
}
