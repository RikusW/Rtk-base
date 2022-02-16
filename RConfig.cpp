// vim:ts=4 sts=0 sw=4
// g++ RFile.cpp RDataCfg.cpp -o t

#include "RConfig.h"
#include <stdio.h>

//-----------------------------------------------------------------------------####

void RConfigNode::GetBin(const char *n, u8 *buf, u32 sz)
{
	RString s,z;
	RConfigNode *t;
	if(!(t = GetNode(n))) {
		return;
	}
	s = t->line;
	z.SplitL(s,'=');

	while(s.l && sz) {
		z.CutL(s,2);
		*buf++ = z.FromHex();
		sz--;
	}
}

//---------------------------------------------------------

u32 RConfigNode::GetValue(const char *n, u32 df)
{
	RString s,z;
	RConfigNode *t;
	if(!(t = GetNode(n))) {
		return df;
	}
	s = t->line;
	z.SplitL(s,'=');

	return (u32)s;
}

//---------------------------------------------------------

u32 RConfigNode::GetValue2(const char *n)
{
	RString s,z;
	RConfigNode *t;
	if(!(t = GetNode(n))) {
		return 0;
	}
	s = t->line;
	z.SplitL(s,'=');
	z.SplitL(s,',');

	return (u32)s;
}

//-----------------------------------------------------------------------------####
// RConfig

int RConfig::Read(const char *c)
{
	int r = RConfig_P::Read(c);
	if(r) {
		return r;
	}
	s = p = (char*)pmem;
	e = s + size;
	return 0;
}

//---------------------------------------------------------

RConfigNode *RConfig::ReadRcf(const char *c)
{
	RString s,ss;
	u32 t,in=0;

	Read(c);
	RConfigNode *p1 = new RConfigNode(0,c);
	RConfigNode *n=p1;
	
	while(GetLine(s)) {
		if(s.Char(0) == '#' || s.l == 0) {
			continue;
		}

		for(t=0; t<100; t++) {
			if(s.Char(t) != '\t') break;
		}
		

		if(t > in) {
			if(t != in+1) {
				printf("parse error\n");
			}
			in++;
			n = (RConfigNode*)n->last;
			if(!n) {
				printf("parse error!!!\n");
				return p1;
			}
		}

		while(t < in) {
			in--;
			n = (RConfigNode*)n->parent;
		}

		s.Trim();
		new RConfigNode(n,s);
	}
	return p1;
}

//---------------------------------------------------------

int RConfig::GetLine(RString &rs)
{
	rs.l = 0;
	rs.p = p;
	if(p >= e)
		return 0;

	for(; p < e; p++,rs.l++) {
		if(*p == 0x0A) {
			if(p > s && *(p-1) == 0x0D) //CRLF
				rs.l--;
			p++;
			break;
		}
		if(*p == '#') { // comment
			while(p < e && *p++ != 0x0A);
			break;
		}
	}
	*(rs.p + rs.l) = '0';
	return 1;
}

// RConfig
//-----------------------------------------------------------------------------####
// RDataCfg

int RDataCfg::Write(const char *)
{
	printf("RDataCfg::Write not currently supported.\n");
	return 1;
}

//---------------------------------------------------------

int RDataCfg::ReadCfg(const char *c)
{
	int r = RDataCfg_P::Read(c);
	if(r)
		return r;
	s = p = (char*)pmem;
	e = s + size;
//	Parse(1);
	return 0;
}

//---------------------------------------------------------

void RDataCfg::Reset()
{
	p = s;
}

//---------------------------------------------------------

int RDataCfg::GetLine(RString &rs)
{
	rs.l = 0;
	rs.p = p;
	if(p >= e)
		return 0;

	for(; p < e; p++,rs.l++) {
		if(*p == 0x0A) {
			if(p > s && *(p-1) == 0x0D) //CRLF
				rs.l--;
			p++;
			break;
		}
		if(*p == '#') { // comment
			while(p < e && *p++ != 0x0A);
			break;
		}
	}
	*(rs.p + rs.l) = '0';
	return 1;
}

//---------------------------------------------------------

void RDataCfg::Parse(int lvl)
{
	int j;
	char buf[20];
	RString a,b;

	for(j=0; j<lvl; j++) {
		buf[j] = '>';
	}
	buf[j] = 0;

	while(GetLine(a)) {
		a.Trim();
		if(a.l == 0)
			continue;
		switch(a.Char(0)) {
		case '{': Parse(lvl+1); continue;
		case '}': return;
		}
		a.SplitL(b,'=');

		printf("x%s %s => %s <=\n",buf,a.GetStr(),b.GetStr());
	}
	if(lvl > 1)
		printf("missing }\n");
}

// RDataCfg
//-----------------------------------------------------------------------------####

/*
int main()
{
	RDataCfg r;
	r.ReadCfg("test.cfg");
	return 0;
}*/


