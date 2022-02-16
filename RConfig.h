// vim:ts=4 sts=0 sw=4

#ifndef __RDATACFG_H__
#define __RDATACFG_H__

#include "RFile.h"
#include "RString.h"
#include "RTreeNode.h"

class RConfigNode : public RTreeNode
{
public:
	 RConfigNode();
	 RConfigNode(RConfigNode *p, const char *cc = "ABC") { Init(p,cc); };
	~RConfigNode() {};

	RConfigNode *GetNode(int i) { return (RConfigNode*)RTreeNode::GetNode(i); };
	RConfigNode *GetNode(const char *c) { return (RConfigNode*)RTreeNode::GetNode(c); };

	void GetBin(const char*, u8*, u32);
	u32 GetValue (const char*, u32 df = 0);
	u32 GetValue2(const char*);
};

#define RConfig_P RData
class RConfig : public RData
{
public:
	RConfig() {};
	~RConfig() {};

	int Read(const char *c);
	//int Write(const char *c);
	RConfigNode *ReadRcf(const char *c);
	int GetLine(RString &rs);
protected:
	char *s,*p,*e;
};

#define RDataCfg_P RData
class RDataCfg : public RData
{
public:
	RDataCfg() {};
	~RDataCfg() {};

	int Write(const char *c);
	int ReadCfg(const char *c);
	
	RTreeNode *ReadRcf(const char *c);

	void Reset();
	int GetLine(RString &rs);
	void Parse(int lvl);
protected:
	char *s,*p,*e;
};

#endif //__RDATACFG_H__

