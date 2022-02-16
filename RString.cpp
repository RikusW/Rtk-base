// vim:ts=4 sts=0 sw=4

#include "RString.h"

//---------------------------------------------------------------####
// RString

RString::RString(const char *c)
{
	p = (char*)c;
	cow = 1;
	for(l = 0; *c++; l++);
}

RString& RString::operator=(const char *c)
{
	p = (char*)c;
	cow = 1;
	for(l = 0; *c++; l++);
	return *this;
}

RString& RString::operator=(const RString& rhs)
{
	l = rhs.l;
	p = rhs.p;
	cow = rhs.cow;
	return *this;
}

void RString::Copy()
{
	if(cow) {
		cow = 0;
		p = GetStr();
	}
}

//---------------------------------------------------------

char RString::Char(u32 i)
{
	return p && (i < l) ? *(p+i) : 0;
}

void RString::Char(u32 i, char c)
{
	//if(cow)  copy
	
	if(i < l) {
		*(p+i) = c;
	}
}

//---------------------------------------------------------

bool RString::operator==(const RString &rhs)
{
	return Cmp(rhs);
}

bool RString::operator==(const char *r)
{
	return Cmp(r);
}

//---------------------------------------------------------

bool RString::Cmp(const RString &rhs, char t)
{
	u32 ll = l,rl = rhs.l;
	char *lp = p,*rp = rhs.p;

	for(; ll && rl && *lp == *rp && *rp != t; ll--, rl--, lp++, rp++);

	if(ll || (rl && *rp != t)) {
		return false;
	}
	return true;
}

bool RString::Cmp(const char *r, char t)
{
	u32 ll;
	char *pp;
	for(ll = l, pp = p; ll && *r != t && *pp == *r; ll--, r++, pp++);
	if(ll || *r != t) {
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool RString::Contains(const RString &rhs)
{
	return false;
}

bool RString::Contains(const char *r)
{
	u32 ll;
	const char *pp,*rr;
	for(ll = l, pp = p, rr = r; ll && *rr; ll--) {
		if(*rr == *pp++) {
			rr++;
		}else{
			rr = r;
		}
	}
	return !*rr;
}

//---------------------------------------------------------

void RString::CopyL(RString &src, u32 u)
{
	if(u > src.l) {
		u = src.l;
	}
	l = u;
	p = src.p;
	cow = 1;
	src.cow = 1;
}

void RString::CopyR(RString &src, u32 u)
{
	if(u > src.l) {
		u = src.l;
	}
	l = u;
	p = src.p + (src.l - u);
	cow = 1;
	src.cow = 1;
}

//---------------------------------------------------------

void RString::CutL(RString &src, u32 u)
{
	if(u > src.l) {
		u = src.l;
	}
	l = u;
	p = src.p;
	cow = 1;

	src.p += u;
	src.l -= u;
}

void RString::CutR(RString &src, u32 u)
{
	if(u > src.l) {
		u = src.l;
	}
	l = u;
	p = src.p + (src.l - u);
	cow = 1;
	src.l -= u;
}

//---------------------------------------------------------

void RString::Mid(RString &src, u32 r, u32 z)
{
	if(r > src.l) {
		src = "";
		*this = "";
		return;
	}

	l = src.l - r;
	p = src.p + r;

	if(l > z) {
		l = z;
	}
}

//---------------------------------------------------------

void RString::SplitL(RString &src, char c)
{
	for(p = src.p, l = 0; src.l; src.l--, l++) { //return right
		if(*src.p++ == c) {
			src.l--;
			break;
		}
	}
}

void RString::SplitR(RString &src, char c)
{
	for(p = src.p + src.l, l = 0; src.l; p--, l++, src.l--) {
		if(*(p-1) == c) {
			src.l--;
			break;
		}
	}
}

//---------------------------------------------------------

void RString::TrimL()
{
	if(!p) return;
	while(l && (*p == ' ' || *p == '\t')) {
		p++; l--;
	}
}

void RString::TrimR()
{
	if(!p) return;
	while(l && (*(p+l-1) == ' ' || *(p+l-1) == '\t')) l--;
}

//---------------------------------------------------------

void RString::Replace(char s, char d)
{
	for(char *c = p; c < (p + l); c++) {
		if(*c == s) *c = d;
	}
}

void RString::Merge(char m)
{
	char *s,*d,*e = p + l;
	s = d = p;
	while(s < e) {
		if(*s == m) {
			while(s < e && *s == m) {
				s++;
				l--;
			}
			l++;
			*d++ = m;
		}else{
			*d++ = *s++;
		}
	}
}

//---------------------------------------------------------

void RString::Term()
{
	p[l] = 0; //XXX cow
}

void RString::ToUpper()
{
	for(char *s = p,*m = p+l; s < m; s++) {
		if(*s >= 'a' && *s <= 'z') {
			*s &= ~0x20;
		}
	}
}

void RString::ToLower()
{
	for(char *s = p,*m = p+l; s < m; s++) {
		if(*s >= 'A' && *s <= 'Z') {
			*s |= 0x20;
		}
	}
}

//---------------------------------------------------------

RString &RString::operator+=(RString &rhs)
{
	char *r = rhs.p;
	u32 rl = rhs.l;

	if(cow) Copy();

	char *e = p+l;

	if(!p || !r) {
		return *this;
	}
	
	l += rl;
	while(rl) {
		*e++ = *r++;
		rl--;
	}
	return *this;
}

RString &RString::operator+=(const char *c)
{
	if(cow) Copy();

	char *e = p+l;

	if(!p || !c) {
		return *this;
	}

	while(*c) {
		*e++ = *c++;
		l++;
	}
	return *this;
}

//---------------------------------------------------------

RString::operator u32()
{
	char *s = p;

	if(*s == '$') {
		return FromHex();
	}

	if(*s == '0') {
		if(*++s == 'x') {
			return FromHex();
		}else{
			return FromOct();
		}
	}

	return FromDec();
}

//---------------------------------------------------------

u32 RString::FromOct()
{
	u32 u = 0;
	char *s = p;

	for(u32 ll = l; *s >= '0' && *s <= '7' && ll; s++, ll--) {
		u <<= 3;
		u |= *s - '0';
	}
	return u;
}

u32 RString::FromDec()
{
	u32 u = 0;
	char *s = p;

	for(u32 ll = l; *s >= '0' && *s <= '9' && ll; s++, ll--) {
		u *= 10;
		u += *s - '0';
	}
	return u;
}

u32 RString::FromHex()
{
	u32 u = 0;
	char *s = p;

	if(s[0] == '$') s++;
	if(s[0] == '0' && s[1] == 'x') s+=2;
	
	for(u32 ll = l; ll; s++, ll--) {
		if(*s >= '0' && *s <= '9') {
			u <<= 4;
			u |= *s - '0';
		}else
		if(*s >= 'A' && *s <= 'F') {
			u <<= 4;
			u |= (*s - 'A') + 10;
		}else
		if(*s >= 'a' && *s <= 'f') {
			u <<= 4;
			u |= (*s - 'a') + 10;
		}else{
			break;
		}
	}
	return u;
}

//---------------------------------------------------------

void RString::ToOct(u32 u)
{
	p = rsbuf+20; //XXX
	*p-- = 0;
	if(!u) *p-- = '0'; else
	while(u) {
		*p-- = '0' + (u & 7);
		u >>= 3;
	}
	*p = '0';
	//XXX l
	l = rsbuf - p + 20;
}

void RString::ToDec(u32 u)
{
	p = rsbuf+20; //XXX
	*p-- = 0;
	if(!u) *p-- = '0'; else
	while(u) {
		*p-- = '0' + (u % 10);
		u /= 10;
	}
	p++;
	l = rsbuf - p + 20;
}

void RString::ToHex(u32 u)
{
	p = rsbuf+20; //XXX
	*p-- = 0;
	if(!u) *p-- = '0'; else
	while(u) {
		if((u & 15) < 10) {
			*p-- = '0' + (u & 15);
		}else{
			*p-- = ('A' - 10) + (u & 15);
		}
		u >>= 4;
	}
	*p-- = 'x';
	*p = '0';
	//XXX l
	l = rsbuf - p + 20;
}

void RString::ToHexN(u32 u, u8 n)
{
	if(n > 8) n = 8;
	l = n;
	p = rsbuf+n; //XXX
	*p = 0;
	while(n) {
		if((u & 15) < 10) {
			*--p = '0' + (u & 15);
		}else{
			*--p = ('A' - 10) + (u & 15);
		}
		u >>= 4;
		n--;
	}
	//XXX l
}

void RString::ToHed(u32 u)
{
	p = rsbuf+20; //XXX
	*p-- = 0;
	if(!u) *p-- = '0'; else
	while(u) {
		if((u & 15) < 10) {
			*p-- = '0' + (u & 15);
		}else{
			*p-- = ('A' - 10) + (u & 15);
		}
		u >>= 4;
	}
	*p = '$';
	//XXX l
	l = rsbuf - p + 20;
}

//---------------------------------------------------------




/*char RString::operator[](int i)
{
	u32 u = (u32)i;
	return p && (u < l) ? *(p+u) : 0;
}*/

char *RString::GetStr()
{
	int i,m = l > 1023 ? 1023 : l;

	//if(cow) ???

	if(p) { //XXX
		for(i = 0; i < m; i++)
			rsbuf[i] = p[i];
		rsbuf[i] = 0;
	}else{
		rsbuf[0] = 0;
	}
	return rsbuf;
}

void RString::SetStr(RString *ss)
{
	p = ss->p;
	l = ss->l;
}

void RString::SetStr(char *ss, u32 ll)
{
	if(!ss) {
		p = (char*)"";
		l = 0;
		return;
	}
	p = ss;
	l = ll;

/*	if(l > 0x10000000)
		printf("RString too long = 0x%x\n",(unsigned int)l);*/
}

//---------------------------------------------------------



/*void RString::Split(RString &s, char c)
{
	s.SetStr("",0);
	u32 ll = l;

	for(l = 0; *(p+l) != c; l++) {
		if(l >= ll)
			return;
	}
	s.SetStr((p + (l + 1)), ll - (l + 1));
}*/

/*
RString& RString::Split(char c)
{
	RString s; //XXX
	for(s.p = p, s.l = 0, s.cow = 1; l && *p++ != c; l--, s.l++); //return left
	return s;
}*/

// RString
//---------------------------------------------------------------####

