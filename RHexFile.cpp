// vim:ts=4 sts=0 sw=4


#include "RConfig.h"
#include "RHexFile.h"
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------

RHexFile::RHexFile(u32 size)
{
	max = 0;
	bufsize = size;
	pmem = malloc(bufsize);
	buf = (u8*)pmem;
};

//-------------------------------------

RHexFile::~RHexFile()
{
	free(pmem);
};

//-------------------------------------

void RHexFile::Clear()
{
	u8 *p = buf;

	for(u32 u = 0; u < bufsize; u++) {
		*p++ = 0xFF;
	}
}

//-----------------------------------------------------------------------------

int RHexFile::Read(const char *name)
{
	u32 u;

	if(!pmem) {
		printf("RHexFile malloc failed\n");
		return -10;
	}

	Clear();

	RConfig dt;
	int r = dt.Read(name);
	if(r) {
		printf("Reading file failed\n");
		return r;
	}

	max = 0;
	RString s,t;
	u32 addresshi = 0;

	while(dt.GetLine(s)) {
		u8 line[270];
		u32 address;

		t.CutL(s,1);
		if(t.Char(0) != ':') {
			printf("Expected : at the start of the line\n");
			return -2;
		}
		t.CutL(s,2);
		u8 count = t.FromHex();
		line[0] = count;
		u8 checksum = count;
		u8 cnt = count + 5;

		for(u = 1; u < cnt; u++) { // count address2 type checksum
			t.CutL(s,2);
			line[u] = t.FromHex();
			checksum += line[u];
		}

		if(checksum != 0) {
			printf("RHexFile checksum fail\n");
			return -5;
		}

		switch(line[3]) { //type
		case 0: //data
			address  = line[1];
			address <<= 8;
			address |= line[2];
			address += addresshi;
			for(u = 0; u < count; u++,address++) {
				buf[address] = line[u+4];
				if(address > max) {
					max = address;
				}
			}
			break;
		case 1: //EOF
			return 0;
		case 2: //segment
			addresshi  = line[4]; //assumed to be BE
			addresshi <<= 8;
			addresshi |= line[5];
			addresshi <<= 4;
			break;
		default:
			printf("RHexFile unsupported record type %02X\n",line[3]);
			return -5;
		}
	}
	printf("RHexFile expected a EOF record\n");
	return -5;
}

//-----------------------------------------------------------------------------

//char tohex[] = "0123456789ABCDEF";

int RHexFile::Write(const char *name)
{
	if(!pmem) {
		printf("RHexFile malloc failed\n");
		return -10;
	}
	
//	u8 *p;
	u32 u,i;
	u8 checksum;
	u32 addresshi = 0; //ignore seg=0;

	FILE *f = fopen(name,"w");
	if(!f) {
		printf("HexFile Write fopen failed\n");
		return -10;
	}

//	for(u = 0, p = buf; u <= max; u+=16) {
	for(u = 0/*, p = buf*/; u < bufsize; u+=16) {
		for(i = 0; i < 16; i++) {
			if(buf[u+i] != 0xFF) {
				goto writedt;
			}
		}
		continue;
writedt:
		if(addresshi != (u & 0xF0000)) { //write segment address
			addresshi = u & 0xF0000;
			checksum = 4 + ((u >> 12) & 0xFF) + ((u >> 4) & 0xFF);
			fprintf(f,":02000002%02X%02X%02X\n",(uint)((u >> 12) & 0xFF), (uint)((u >> 4) & 0xFF), (-checksum & 0xFF));
		}

		checksum = 0x10 + ((u >> 8) & 0xFF) + (u & 0xFF);
		for(i = 0; i < 16; i++) {
			checksum += buf[u+i];
		}
		fprintf(f,":10%02X%02X00%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
			(uint)((u >> 8) & 0xFF), (uint)(u & 0xFF),
			buf[u + 0],buf[u + 1],buf[u + 2],buf[u + 3],
			buf[u + 4],buf[u + 5],buf[u + 6],buf[u + 7],
			buf[u + 8],buf[u + 9],buf[u +10],buf[u +11], 
			buf[u +12],buf[u +13],buf[u +14],buf[u +15], (-checksum & 0xFF));
	}

	fprintf(f,":00000001FF\n"); //EOF
	fclose(f);
	return 0; //XXX ??
}

//-----------------------------------------------------------------------------

