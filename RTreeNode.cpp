// vim:ts=4 sts=0 sw=4

#include <string.h>
#include "RTreeNode.h"

//---------------------------------------------------------------------------------------

void RTreeNode::Init(RTreeNode *p, const char *cc)
{
	prev = next = 0; bExpand = false;
	child = last = parent = 0; *line = 0;

	if(!p) {
		bExpand = true; // root node
	}

	if((parent = p)) {
	if(parent->child) {
		prev = parent->last;
		prev->next = this;
		parent->last = this;
	}else{
		parent->child = parent->last = this;
	}}
	SetText(cc);
};

RTreeNode::~RTreeNode()
{
	if(next) delete next;
	if(child) delete child;
};

int RTreeNode::GetDepth(void)
{
	int i = 0;
	for(RTreeNode *p = this; p->parent; p = p->parent, i++);
	return i;
}

RTreeNode *RTreeNode::GetPrev()
{
	RTreeNode *t = this;
	if(prev) {
		for(t = prev; t->last && t->bExpand; t = t->last);
	}else{
		if(parent) {
			t = parent;
		}
	}
	return t;
}

RTreeNode *RTreeNode::GetNext()
{
	RTreeNode *t = this;
	if(child && bExpand) {
		 t = child;
	}else{
		while(!t->next) {
			t = t->parent;
			if(!t) return this; //root node
		}
		t = t->next;
	}
	return t;
}

RTreeNode *RTreeNode::GetNode(int n)
{
	RTreeNode *s,*t = this;
	for(int i = 0; i < n && t != (s = t->GetNext()); i++, t = s);
	return t;
}

#include "RString.h" //XXX

RTreeNode *RTreeNode::GetNode(const char *c) //XXX
{
	RString s(c),d,r,rr;

	RTreeNode *t = child;
	d.SplitL(s,'/');
	while(t) {
		rr = t->line;
		r.SplitL(rr,'=');
		if(d == r) {
			d.SplitL(s,'/');
			if(!d.l) {
				break;
			}
			t = t -> child;
			continue;
		}

		t = t->next;
	}
	return t;
}

int RTreeNode::GetNumber()
{
	int i = 0;
	for(RTreeNode *s,*t = this; t != (s = t->GetPrev()); t = s, i++);
	return i;
}

// currently 0 based...
int RTreeNode::Count(RTreeNode *stop)
{
	int i = 0;
	for(RTreeNode *s=0,*t = this; t != (s = t->GetNext()) && t != stop; i++, t = s);
	return i;
}

bool RTreeNode::IsChild(RTreeNode *s,bool rec)
{
	RTreeNode *n = child;
	while(n) {
		if(n == s || (rec && n->IsChild(s,true)))
			return true;
		n = n->next;
	}
	return false;
};

void RTreeNode::SetText(const char *c)
{
	strncpy(line,c,80);
	line[80] = 0;
}

void RTreeNode::Sort()
{
	if(!child) {
		return;
	}

	RTreeNode *n,*s,*t,*l;
	for(n = child; n; n = n->next) n->Sort();

	l = child->next;
	child->next = 0;
	s = child;
	while(l) {
		n = l;
		l = l->next;
		for(t = s;; t = t->next) {
			if(strcmp(n->line, t->line) < 0) {
				n->prev = t->prev;
				n->next = t;
				t->prev = n;
				if(n->prev) {
					n->prev->next = n;
				}else{
					s = n;
				}
				break;
			}
			if(!t->next) {
				t->next = n;
				n->prev = t;
				n->next = 0;
				break;
			}
			
		}
	}

	for(child = s; s->next; s = s->next);
	last = s;
}

//---------------------------------------------------------------------------------------

