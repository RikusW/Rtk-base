// vim:ts=4 sts=0 sw=4

#ifndef TREENODE_H
#define TREENODE_H

class RTreeNode
{
public:
	RTreeNode() {};
	void Init(RTreeNode *p, const char *cc = "ABC");
	RTreeNode(RTreeNode *p, const char *cc = "ABC") { Init(p,cc); };
	virtual ~RTreeNode();
	int GetDepth(void);
	RTreeNode *GetPrev();
	RTreeNode *GetNext();
	virtual RTreeNode *GetChild() { return child; };
	RTreeNode *GetNode(int);
	RTreeNode *GetNode(const char *);
	void Sort();
	int GetNumber();
	int Count(RTreeNode *stop = 0);
	bool IsChild(RTreeNode *s,bool rec);
	void SetText(const char *c);

	RTreeNode *prev;
	RTreeNode *next;
	RTreeNode *child;
	RTreeNode *last;
	RTreeNode *parent;
	char line[300];
	bool bExpand;
};

#endif //TREENODE_H

