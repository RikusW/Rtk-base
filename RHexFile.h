// vim:ts=4 sts=0 sw=4

#ifndef RHEXFILE_H
#define RHEXFILE_H

#include "rtypes.h"

class RHexFile
{
public:
	RHexFile(u32 bufsize);
	~RHexFile();

	void Clear();
	int Read(const char *name);
	int Write(const char *name);

	u8 *buf;
	void *pmem;
	u32 bufsize;
	u32 max;
};

#endif //RHEXFILE_H
