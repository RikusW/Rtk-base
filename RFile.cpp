// vim:ts=4 sts=0 sw=4

#include "RFile.h"

#define FORCE32 //for macosx XXX ???!!!

#ifndef _WIN32
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <sys/types.h>//open + lseek
#include <sys/stat.h>// open
#include <fcntl.h>   // open
#include <unistd.h>  // read + write + lseek + ftruncate
#include <stdlib.h>  // malloc + free
#include <sys/mman.h>// mmap/munmap
#include <string.h>  //strlen/strncpy
#include <stdio.h>  //printf
#include <errno.h>
#else
#include <windows.h>
#include <stdio.h> //printf
#endif

//-----------------------------------------------------------------------------
// RData Definition

/**
 * Allocate Buffer
 * @sz
 * Size of the Buffer
 * @@
 * 0 = Success
 */
int RData::Alloc(u32 sz)
{
	Free();
	pmem = malloc(sz);
	if(!pmem) {
		printf("\n----MALLOC FAIL----\n\n");
		return 1;
	}
	autofree = true;
	size = sz;
	return 0;
}

/**
 * Free Buffer
 */
void RData::Free()
{
	if(!autofree) return;
	if(pmem) free(pmem);
	pmem = 0;
	size = 0;
}

/**
 * Read data from a file
 * @c
 * The filename
 * @@
 * 0 = Success
 */
int RData::Read(const char *c)
{
	RFile f;
	return f.ReadAll(c,this);
}

/*
 * Write data to a file
 * @c
 * The Filename
 * @@
 * 0 = Success
 */
int RData::Write(const char *c)
{
	RFile f;
	return f.WriteAll(c,this);
}

// RData Definition
//-----------------------------------------------------------------------------
// RFile definition

int RFile::PageSize;

// start Linux
#ifndef _WIN32

RFile::RFile()
{
	fd = 0; size = 0; fn = 0;
	mm_ptr = 0; mm_len = 0;
	PageSize = getpagesize();
}

/**
 * Open a file
 * @p
 * The filename
 * @rw
 * Can be RF_R, RF_W or RF_RW
 * @x
 * Can be:
 * RF_OPEN   1---
 * RF_CREATE ---4
 * RF_TRUNC  1-3-
 * RF_CREATT --34
 * RF_NEW    -234
 *
 * 1: File must exist
 * 2: File must not exist
 * 3: Opened file will be empty
 * 4: File will be created if it don't exist
 * @@
 *  0 = Success
 * -1 = Failure
 */
int RFile::Open(const char *p, int rw, int x)
{
	Close();

	switch(rw) {
	case RF_R:  rw = O_RDONLY; break;
	case RF_W:  rw = O_WRONLY; break;
	case RF_RW: rw = O_RDWR; break;
	default:
		printf("RFile::Open param 2 invalid\n");
		return -1;
	}

	switch(x) {
	case RF_OPEN:   x = 0; break;
	case RF_CREATE: x = O_CREAT; break;
	case RF_TRUNC:  x = O_TRUNC; break;
	case RF_CREATT: x = O_CREAT | O_TRUNC; break;
	case RF_NEW:    x = O_CREAT | O_EXCL; break;
	default:
		printf("RFile::Open param 3 invalid\n");
		return -1;
	}

#ifdef FORCE32
	fd = open(p,rw|x,0664);//|O_LARGEFILE,0664);
#else
	fd = open64(p,rw|x,0664);//|O_LARGEFILE,0664);
#endif
	if(fd == -1) {
		fd = 0;
		printf("open %s fail = %i\n",p,errno);
		return -1;
	}
	
#ifdef FORCE32
	struct stat st;
	if(fstat(fd,&st)) {
		printf("fstat %s fail\n",p);
		return -1;
	}
	size = st.st_size;
#else
	struct stat64 st;
	if(fstat64(fd,&st)) {
		printf("fstat %s fail\n",p);
		return -1;
	}
	size = st.st_size;
#endif

	int l = strlen(p) + 1;
	fn = (char*)malloc(l);
	strncpy(fn,p,l);
	
	return 0;
}

/**
 * Close a file
 * @@
 *  0 = Success
 * -1 = Failure
 */
int RFile::Close()
{
	int i = 0;
	MUnmap();
	if(fd) {
		i = close(fd);
		fd = 0;
	}
	if(fn) {
		free(fn);
		fn = 0;
	}

	return i;
}

/**
 * Set the end of file and set the file pointer to EOF as well
 * @len
 * Number of bytes the file should have afterward
 * @@
 *  0 = Success
 * -1 = Failure
 */
int RFile::SetEOF(s64 len)
{
	size = len;
	if(Seek64(len,RF_SEEKSET) != -1) {
#ifdef FORCE32
		return ftruncate(fd,len);
#else
		return ftruncate64(fd,len);
#endif
	}
	return -1;
}

inline int SeekTr(u32 &wh)
{
	switch(wh) {
	case RF_SEEKSET: wh = SEEK_SET; break;
	case RF_SEEKCUR: wh = SEEK_CUR; break;
	case RF_SEEKEND: wh = SEEK_END; break;
	default:
		printf("RFile::Seek param 2 invalid\n");
		return -1;
	}
	return 0;
}

/**
 * Set the file pointer
 * @off
 * Offset
 * @wh
 * Can be:
 * RF_SEEKSET absolute offset
 * RF_SEEKCUR add to file pointer
 * RF_SEEKEND add to end of file
 * @@
 * Returns the absolute offset
 * -1 = Error
 */
int RFile::Seek(s32 offs, u32 wh)
{
	if(SeekTr(wh) == -1)
		return -1;
	return lseek(fd,offs,wh);
};

/**
 * Same as Seek except for offs = 64bit
 * @@
 * 0  = Success
 * -1 = Error
 */
int RFile::Seek64(s64 offs, u32 wh)
{
	if(SeekTr(wh) == -1)
		return -1;
#ifdef FORCE32
	return lseek(fd,offs,wh) & 0xFFFFFFFF; //XXX TODO
#else
	return lseek64(fd,offs,wh) & 0xFFFFFFFF; //XXX TODO
#endif
}

//---------------------------
/*
int errno;
#define __KERNEL__
#include <asm/unistd.h>
_syscall5(int, _llseek, uint, fd, ulong, hi, ulong, lo, loff_t *, res, uint, wh);

int RFile::Seek64(u64 offs, u32 wh)
{
	loff_t res;
	if(SeekTr(wh) == -1)
		return -1;
	u32 hi = (offs >> 32) & 0xFFFFFFFF, lo = offs & 0xFFFFFFFF;
	return _llseek(fd, hi, lo, &res, wh); //XXX
}/*/
//---------------------------

/**
 * Read file into buffer
 * @buf
 * Pointer to buffer
 * @cnt
 * Number of bytes to be read
 * @@
 * Number of byte read
 * -1 = Error
 */
int RFile::Read(void *buf, u32 cnt)
{
	return read(fd,buf,cnt); // ret # of bytes or -1   TODO partial read ?
}

/**
 * Write buffer into file
 * @buf
 * Pointer to buffer
 * @cnt
 * Number of bytes to be written
 * @@
 * Number of byte written
 * -1 = Error
 */
int RFile::Write(void *buf, u32 cnt)
{
	return write(fd,buf,cnt);
}

/**
 * Memory map file contents
 * @f
 * Flags, can be a combination of:
 * MMAP_R read
 * MMAP_W write
 * MMAP_X execute
 * MMAP_C Copy on write, else it is shared
 * @offs
 * Offset into the file in units of PageSize (typically 4096)
 * @len
 * Number of bytes to map
 * @@
 * Return a pointer
 * 0 = Error
 */
void *RFile::MMap(u32 f, u32 offs, u32 len)
{
	void *p;
	s32 prot = 0, flags = 0;
	u64 o = offs;
	o *= PageSize;

	MUnmap();
//	if(mm_ptr)
//		return 0;

//	if(offs % PageSize)
//		return 0;

	if(len == 0) len = size;
    if(len + offs > (u32)size) //x86x64
		len -= (len + offs) - size;
	if(f & MMAP_R) prot |= PROT_READ;
	if(f & MMAP_W) prot |= PROT_WRITE;
	if(f & MMAP_X) prot |= PROT_EXEC;
	if(f & MMAP_C) flags|= MAP_PRIVATE; // copy on write ...cow...
		else flags |= MAP_SHARED;

#ifdef FORCE32
	if((p = mmap(0,len,prot,flags,fd,o)) == MAP_FAILED)
#else
	if((p = mmap64(0,len,prot,flags,fd,o)) == MAP_FAILED)
#endif
		return 0;
	mm_ptr = p;
	mm_len = len;
	return p;
}

/**
 * Write a mapping back to disk
 * @p
 * Start of mapping
 * @l
 * Length of mapping
 * @@
 *  0 = Success
 * -1 = Error
 */
int RFile::MSync(void *p , int l)
{
	if(!p) p = mm_ptr;
	if(!l) l = mm_len;

	// bounds check
	char *a = (char*)mm_ptr, *b = (char*)p;
	a += mm_len; b += l;
	if(p < mm_ptr || b > a)
		return 1;
	
	return msync(p,l,MS_SYNC);
}

/**
 * Unmap a file
 * @@
 *  0 = Success
 * -1 = Error
 */
int RFile::MUnmap()
{
	if(!mm_ptr)
		return -1;

	int ret =  munmap(mm_ptr,mm_len);
	mm_ptr = 0;
	mm_len = 0;
    return ret;
}

/**
 * Get the filetime
 * @a
 * Access time
 * @m
 * Modification time
 * @@
 * 0 = Success
 */
int RFile::GetTime(u32 *a, u32 *m)
{
	struct stat st;
	if(fstat(fd,&st)) {
		printf("fstat %s fail\n","fn fixme");
		return 1;
	}

	if(a) *a = st.st_atime;
	if(m) *m = st.st_mtime;
	return 0;
}

#include <utime.h>

/**
 * Set the filetime
 * @a
 * Access time
 * @m
 * Modification time
 * @@
 *  0 = Success
 * -1 = Error
 */
int RFile::SetTime(u32 *a, u32 *m)
{
	u32 A,M;
	GetTime(&A,&M);
	if(!a) a = &A;
	if(!m) m = &M;

	utimbuf tb;
	tb.actime = *a;
	tb.modtime = *m;

	return utime(fn,&tb);
}

#endif

// end Linux
//-----------------------------------------------------------------------------
// begin Windows

#ifdef WIN32

RFile::RFile()
{
	hFM = 0;
	fd = 0; size = 0; fn = 0;
	mm_ptr = 0; mm_len = 0;
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	PageSize = si.dwPageSize;
}

int RFile::Open(const char *p, int rw, int x)
{
	Close();

	switch(rw) {
	case RF_R:  rw = GENERIC_READ; break;
	case RF_W:  rw = GENERIC_WRITE; break;
	case RF_RW: rw = GENERIC_READ | GENERIC_WRITE; break;
	default:
		printf("RFile::Open param 2 invalid\n");
		return -1;
	}

	switch(x) {
	case RF_OPEN:   x = OPEN_EXISTING; break;
	case RF_CREATE: x = OPEN_ALWAYS; break;
	case RF_TRUNC:  x = TRUNCATE_EXISTING; break;
	case RF_CREATT: x = CREATE_ALWAYS; break;
	case RF_NEW:    x = CREATE_NEW; break;
	default:
		printf("RFile::Open param 3 invalid\n");
		return -1;
	}

	fd = CreateFile(p,rw,FILE_SHARE_READ | FILE_SHARE_WRITE,0,x,0,0);
	if (fd == INVALID_HANDLE_VALUE)	{
		printf("Cannot open %s\n",p);
		return -1;
	}

	size = GetFileSize(fd,0);
	if(size == -1) {
		size = 0;
		printf("GetFileSize fail\n");
		return -1;
	}

	int l = strlen(p) + 1;
	fn = (char*)malloc(l);
	strncpy(fn,p,l);

	return 0;
}

int RFile::Close()
{
	int i = 0; // OK if already closed
	if(fd) {
		i = CloseHandle(fd) ? 0 : -1;
		fd = 0;
	}
	if(fn) {
		free(fn);
		fn = 0;
	}
	return i;
}

int RFile::SetEOF(s64 len)
{
	size = len;
	if(Seek64(len,RF_SEEKSET) != -1) {
		if(SetEndOfFile(fd)) {
			return 0;
		}
	}
	return -1;
}

static int SeekTr(u32 &wh)
{
	switch(wh) {
	case RF_SEEKSET: wh = FILE_BEGIN; break;
	case RF_SEEKCUR: wh = FILE_CURRENT; break;
	case RF_SEEKEND: wh = FILE_END; break;
	default:
		printf("RFile::Seek param 2 invalid\n");
		return -1;
	}
	return 0;
}

int RFile::Seek(s32 off, u32 wh)
{
	if(SeekTr(wh) == -1)
		return -1;
	return SetFilePointer(fd,off,0,wh);	
}

int RFile::Seek64(s64 offs, u32 wh)
{
	if(SeekTr(wh) == -1)
		return -1;
	long hi = (u32)((offs >> 32) & 0xFFFFFFFF), lo = (u32)(offs & 0xFFFFFFFF);
	if((lo = SetFilePointer(fd,lo,&hi,wh)) == -1) { //INVALID_SET_FILE_POINTER) { //== -1
		if(GetLastError() != NO_ERROR) {
			return -1;
		}
	}
	return lo; //XXX TODO
}

int RFile::Read(void *buf, u32 cnt)
{
	DWORD d;
	if(ReadFile(fd,buf,cnt,&d,0)) return d; else return -1;
}

int RFile::Write(void *buf, u32 cnt)
{
	DWORD d;
	if(WriteFile(fd,buf,cnt,&d,0)) return d; else return -1;
}

void *RFile::MMap(u32 f, u32 offs, u32 len)
{
	void *p;
	s32 prot = 0;
	u64 o = offs;
	o *= PageSize;

	MUnmap();
//	if(mm_ptr)
//		return 0;
	
	if(f & MMAP_R) prot = PAGE_READONLY;
	if(f & MMAP_W) prot = PAGE_READWRITE;
	if(f & MMAP_C) prot = PAGE_WRITECOPY;
	hFM = CreateFileMapping(fd,0,prot,0,(u32)size,0);
	if(!hFM) return 0;

	if(len == 0) len = (u32)size;
    if((u32)(len + offs) > (u32)size)
		len -= (len + offs) - (u32)size;
	if(f & MMAP_R) prot = FILE_MAP_READ;
	if(f & MMAP_W) prot = FILE_MAP_WRITE;
	if(f & MMAP_C) prot = FILE_MAP_COPY;

	if(!(p = MapViewOfFile(hFM,prot,(u32)(o >> 32),(u32)o,len))) {
		CloseHandle(hFM);
		hFM = 0;
		return 0;
	}
//	if(f & MMAP_X) --> use VirtualProtect here...
	mm_ptr = p;
	mm_len = len;
	return p;
}

int RFile::MSync(void *p , int l)
{
	if(!p) p = mm_ptr;
	if(!l) l = mm_len;

	// bounds check
	char *a = (char*)mm_ptr, *b = (char*)p;
	a += mm_len; b += l;
	if(p < mm_ptr || b > a)
		return 1;
	
	if(FlushViewOfFile(p,l)) return 0;
	return -1;
}

int RFile::MUnmap()
{
	int ret = 0;
	
    if(!UnmapViewOfFile(mm_ptr)) ret = -1;
	CloseHandle(hFM);
	mm_ptr = 0;
	mm_len = 0;
	hFM = 0;

	return ret;
}

// convert win time to Linux time
u32 LTime(FILETIME &ft)
{
	u64 u;
	u = ft.dwHighDateTime;
	u <<= 32;
	u |= ft.dwLowDateTime;
	u /= 10000000; // 10M  100nSec to 1Sec
	u -= 11644466400; // 1601 to 1970
	return (u32)u;
}

int RFile::GetTime(u32 *a, u32 *m)
{
	FILETIME A,M;
	GetFileTime(fd,0,&A,&M);
	if(a) *a = LTime(A);
	if(m) *m = LTime(M);
	return 0; //XXX
}

// convert Linux time to win time
void WTime(u32 lt, FILETIME &ft)
{
	u64 u = lt;
	u += 11644466400; // 1970 to 1601
	u *= 10000000; // 10M  1Sec to 100nSec
	ft.dwLowDateTime = (u32)(u & 0xFFFFFFFF);
	ft.dwHighDateTime = (u32)((u >> 32) & 0xFFFFFFFF);
}

int RFile::SetTime(u32 *a, u32 *m)
{
	FILETIME A,M;
	GetFileTime(fd,0,&A,&M);
	if(a) WTime(*a,A);
	if(m) WTime(*m,M);
	SetFileTime(fd,0,&A,&M);
	return 0; //XXX
}

#endif

// end Windows
//-------------------------------------

/**
 * Read the entire file into a buffer
 * @p
 * The filename
 * @d
 * RData buffer
 * @@
 * 0 = Success
 */
int RFile::ReadAll(const char *p, RData *d)
{
	// change 10000000 to an editable option
	if(size > 10000000 || Open(p,RF_R,RF_OPEN))
		return 1;

	d->Alloc((u32)size); 
	
	if(Read(d) == -1)
		return 1;
	
	return 0;
}

/**
 * Write a buffer into a file
 * @p
 * The filename
 * @d
 * RData buffer
 * @@
 * 0 = Success
 */
int RFile::WriteAll(const char *p, RData *d)
{
	if(Open(p,RF_W,RF_CREATT))
		return 1;

	if(Write(d) == -1)
		return 1;

	return 0;
}

// RFile definition
//-----------------------------------------------------------------------------
// RDir definition

#ifndef WIN32

#include <dirent.h>

const char *RDir::GetFirst(int *p, const char *cc)
{
	if(dir) {
		Close();
	}
	dir = (void*)opendir(cc);
	if(!dir) {
		printf("opendir fail >%s< %i\n",cc,errno);
		return 0;
	}
	return GetNext(p);
}

const char *RDir::GetNext(int *p)
{
	errno = 0; //XXX
	dirent *de = readdir((DIR*)dir);
	if(!de) {
		if(errno)
			printf("readdir fail %i\n",errno);
		return 0;
	}
	if(p) {
		*p = 0;
		if(p && de->d_type == DT_DIR) {
			*p = RD_DIR;
		}
		char *a = de->d_name;
		if(*a == '.') {
			*p |= RD_HIDDEN;
			if(a[1] == '\0' || (a[1] == '.' && a[2] == '\0')) {
				*p |= RD_DDOT;
				*p &= ~RD_DIR;
			}
		}
	}
	return de->d_name;
}

void RDir::Close()
{
	if(dir && closedir((DIR*)dir)) {
		printf("closedir fail\n");
	}
	dir = 0;
}

#endif
#ifdef WIN32

const char *RDir::GetFirst(int *p, const char *cc)
{
	if(dir) {
		Close();
	}
	WIN32_FIND_DATA wfd;
	strncpy(buf,cc,298); buf[298] = 0;
	strcat(buf,"*"); //JUCK
	if((dir = FindFirstFile(buf,&wfd)) == INVALID_HANDLE_VALUE) {
		//= GetLastError();
		printf("FindFirstFile fail\n");
		return 0;
	}
	return GetRet(p,&wfd);
}

const char *RDir::GetNext(int *p)
{
	WIN32_FIND_DATA wfd;
	if(!FindNextFile(dir,&wfd)) {
		if(GetLastError() != ERROR_NO_MORE_FILES)
			printf("FindFirstFile fail\n");
		return 0;
	}
	return GetRet(p,&wfd);
}

const char *RDir::GetRet(int *p, void *w)
{
	LPWIN32_FIND_DATA pw = (LPWIN32_FIND_DATA)w;
	if(p) {
		*p = 0;
		if(pw->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			*p |= RD_DIR;
		}
		if(pw->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			*p |= RD_HIDDEN;
		}
		char *a = pw->cFileName;
		if(*a == '.') {
			*p |= RD_HIDDEN;
			if(a[1] == '\0' || (a[1] == '.' && a[2] == '\0')) {
				*p |= RD_DDOT;
				*p &= ~RD_DIR;
			}
		}
	}
	strncpy(buf,pw->cFileName,300);
	return buf;
}

void RDir::Close()
{
	if(dir && !FindClose((WIN32_FIND_DATA*)dir)) {
		printf("FindClose fail\n");
	}
	dir = 0;
}

#endif

// RDir definition
//-----------------------------------------------------------------------------
