// vim:ts=4 sts=0 sw=4

#ifndef __RFILE_H__
#define __RFILE_H__

#include "rtypes.h"

//---------------------------
// RData declaration


class RData
{
public:
	RData() { pmem = 0; size = 0; };
	RData(u32 sz) { Alloc(sz); };
	~RData() { Free(); };
	int Read(const char *);
	int Write(const char *);

	void SetMem(void *p, int sz, bool b = false)
	{
		pmem = p; size = sz; autofree = b;
	};

	int Alloc(u32 sz);
	void Free();

	u32 size;
	void *pmem;
	bool autofree;
};

// RData declaration
//---------------------------
// RFile declaration

// Open param 2
#define RF_R	1
#define RF_W	2
#define RF_RW	3

// Open param 3
#define RF_OPEN		0
#define RF_CREATE	1
#define RF_TRUNC	2
#define RF_CREATT	3
#define RF_NEW		5
//--for internal use--
#define RF_CR 1
#define RF_TR 2
#define RF_EX 4
// 4, 6 & 7 is invalid combinations
//--for internal use--

#define RF_SEEKSET 0
#define RF_SEEKCUR 1
#define RF_SEEKEND 2

#define MMAP_R  1
#define MMAP_W  2
#define MMAP_RW 3
// copy on write
#define MMAP_C  4
#define MMAP_X  8

class RFile
{
public:
	RFile();
	RFile(const char *p, int rw = RF_R, int x = RF_OPEN) { RFile(); Open(p,rw,x); };
	~RFile() { Close(); };
	
	int Open(const char *p, int rw = RF_R, int x = RF_OPEN);
	int Close();
	int SetEOF(s64 len);
	int Seek(s32 off, u32 wh = RF_SEEKSET);
	int Seek64(s64 offs, u32 wh = RF_SEEKSET);
	int Read(void *buf, u32 cnt);
	int Write(void *buf, u32 cnt);
	void *MMap(u32 flags = MMAP_RW, u32 offs = 0, u32 len = 0);
	int MSync(void *p = 0, int len = 0);
	int MUnmap();
	int GetTime(u32 *a, u32 *m);
	int SetTime(u32 *a, u32 *m);

	// implement posix_fadvise
	// implement madvise
	// locking ????

	int ReadAll(const char *p, RData *d);
	int WriteAll(const char *p, RData *d);
	inline int Read(RData *d)  { return Read(d->pmem, d->size); };
	inline int Write(RData *d) { return Write(d->pmem,d->size); };
#ifndef _WIN32
	int fd;
#else
	void *fd,*hFM; // type problem, must actually be HANDLE
#endif	
	char *fn;
	u32 mm_len;
	void *mm_ptr;
	s64 size; //XXX file size extension when writing ???
	static int PageSize;
};

// RFile declaration
//---------------------------
// RDir declaration

#define RD_DIR	1
#define RD_DDOT 2
#define RD_HIDDEN 4

class RDir
{
public:
	RDir() { dir = 0; };
	~RDir() { Close(); };

	const char *GetFirst(int*,const char*);
	const char *GetNext(int*);
	void Close();

	void *dir;

#ifdef WIN32
private:
	const char *GetRet(int*, void*);
	char buf[301];
#endif
};

// RDir declaration
//---------------------------

#endif //__RFILE_H__
