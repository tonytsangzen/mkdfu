#ifndef __DFUSE_H__
#define __DFUSE_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define PREFIX_SIGNATURE		"DfuSe"
#define SUFFIX_SIGNATURE		"UFD"
#define TARGET_SIGNATURE		"Target"
#define DFU_VERSION				0x01
#define BCD_DFU					0x11A

/********************************************/
#define BIGENDINA
//#define DEBUG

#ifdef BIGENDINA	
#define ToBig(x)	(x)
#define ToLittle(x)	((x>>24)&0xff) | \
                    ((x<<8)&0xff0000) | \
                    ((x>>8)&0xff00) | \
                    ((x<<24)&0xff000000); 
#else
#define ToBig(x)	((x>>24)&0xff) | \
                    ((x<<8)&0xff0000) | \
                    ((x>>8)&0xff00) | \
                    ((x<<24)&0xff000000); 
#define ToLittle(x)	(x)
#endif

#ifdef DEBUG
	#define DFUPrintf(...) 	printf(__VA_ARGS__)
#else
	#define DFUPrintf(...)   do{}while(0)
#endif

#pragma pack(1)
typedef struct {
	char szSignature[5];
	uint8_t bVersion;
	uint32_t DFUImageSize;
	uint8_t bTargets;
}DfuPrefixType;

typedef struct {
	char szSignature[6];
	uint8_t bAlternateSetting;
	uint32_t bTargetNamed;
	uint8_t szTargetName[255];
	uint32_t dwTargetSize;
	uint32_t dwNbElements;
}DfuTargetPrefixType;

typedef struct {
	uint32_t dwElementAddress;
	uint32_t dwElementSize;
}DfuElementPrefixType;

typedef struct {
	uint8_t bcdDeviceLo;
	uint8_t bcdDeviceHi;
	uint8_t idProductLo;
	uint8_t idProductHi;
	uint8_t idVendorLo;
	uint8_t idVendorHi;
	uint8_t bcdDFULo;
	uint8_t bcdDFUHi;
	uint8_t ucDfuSignature[3];
	uint8_t bLength;
	uint32_t dwCRC;
}DfuSuffixType;
#pragma pack(0)
/***************************/
int Dfu_AddPrefix(DfuPrefixType* prefix, int version);
int Dfu_AddImage(DfuPrefixType* prefix, DfuTargetPrefixType* target, int alt, const char* name);
int Dfu_AddElement(DfuPrefixType* prefix, DfuTargetPrefixType* target, DfuElementPrefixType* element, uint32_t addr,const char* buf, uint32_t size);
int Dfu_AddSuffix(DfuPrefixType* prefix, DfuSuffixType* suffix, int bcd, int pid, int vid);
	
#endif

