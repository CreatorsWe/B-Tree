// B-Tree.cpp: 定义应用程序的入口点。
//

#include "B-Tree.h"


int main()
{
	BTree<int> btree(4,make_pair(3,5));
	BTree<int> btree1(btree);
	cout<<"degree:"<<btree1.getdegree()<<"  "<<"最小关键字数："<<btree1.getdegree() -1<<endl;
	cout<<" order:"<<btree1.getorder()<<"  "<<"最大关键字数："<<btree1.getorder() -1<<endl;
	btree1.insert(4,6);
	btree1.insert(5,7);
	btree1.insert(3,4);
	btree1.insert(9,4);
	btree1.insert(10,5);
	btree1.insert(11,7);
	btree1.insert(1,9);
	btree1.insert(2,5);
	btree1.insert(12,6);
	btree1.insert(13,5);
	btree1.insert(0,4);
	btree1.midorder();
	btree1.sequenceorder();
	cout<<btree1.height()<<endl;
	return 0;
}
