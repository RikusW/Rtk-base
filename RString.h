// vim:ts=4 sts=0 sw=4

#ifndef RSTRING_H
#define RSTRING_H

#include "rtypes.h"

class RString
{
public:
	RString() { p = 0; l = 0; cow = 0; };
	RString(const char *);
	~RString() {};

	RString& operator=(const char *);
	RString& operator=(const RString &rhs);
	RString(const RString& rhs)
	{ *this = rhs; };

	bool operator==(const RString &rhs);
	bool operator==(const char *);
	bool Cmp(const RString &rhs, char t=0);
	bool Cmp(const char *r, char t=0);

	bool Contains(const RString &rhs);
	bool Contains(const char *r);

	char Char(u32 i);
	void Char(u32 i, char c);
//	char operator[](int);

	void CopyL(RString &src, u32 l);
	void CopyR(RString &src, u32 l);
	void CutL(RString &src, u32 l);
	void CutR(RString &src, u32 l);
	void Mid(RString &src, u32 r, u32 l);

	void SplitL(RString &src, char c);
	void SplitR(RString &src, char c);

	void TrimL();
	void TrimR();
	void Trim() { TrimL(); TrimR(); };
	void Replace(char s,char d);
	void Merge(char m);
	void Term();
	void ToUpper();
	void ToLower();
	RString &operator+=(RString &);
	RString &operator+=(const char*);



	operator u32();
	u32 FromOct();
	u32 FromDec();
	u32 FromHex();

	void ToOct(u32);
	void ToDec(u32);
	void ToHex(u32);
	void ToHed(u32);
	void ToHexN(u32,u8);


	char *GetStr();
	operator const char *() { return GetStr(); };
	operator void *() { return GetStr(); };

	void SetStr(RString *ss);
	void SetStr(char *ss, u32 ll);

	void Copy(); //make explicit copy into buffer

	u32 l;
	u8 cow;
	char *p;
	char rsbuf[1024]; //XXX
};

#endif //RSTRING_H

