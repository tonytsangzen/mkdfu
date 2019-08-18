#include "dfuse.h"
#include "crc32.h"

/***************************
* add Prefix
*/
int Dfu_AddPrefix(DfuPrefixType* prefix, int version){
	DFUPrintf("init Prefix...\n");
	DFUPrintf("Prefix Signature: %s\n", PREFIX_SIGNATURE);
	memcpy(prefix->szSignature, PREFIX_SIGNATURE, sizeof(prefix->szSignature));
	DFUPrintf("Prefix Version: %d\n", version);
	prefix->bVersion = version;
	DFUPrintf("Prefix Size: %d\n", 0);
	prefix->DFUImageSize = ToBig(0); 
	return sizeof(DfuPrefixType);
}

/***************************
* add image
*/
int Dfu_AddImage(DfuPrefixType* prefix, DfuTargetPrefixType* target, int alt, const char* name){
	DFUPrintf("init target Prefix...\n");
	DFUPrintf("Target Signature: %s\n", TARGET_SIGNATURE);
	memcpy(target->szSignature, TARGET_SIGNATURE, sizeof(target->szSignature));
	target->bAlternateSetting = alt;
	if(name){
		DFUPrintf("Target name: %s\n", name);
		target->bTargetNamed = 1;
		strncpy((char*)target->szTargetName, name, sizeof(target->szTargetName));
	}
	DFUPrintf("Target Size: %d\n", 0);
	target->dwTargetSize = ToBig(0);
	DFUPrintf("Target Elements: %d\n", 0);
	target->dwNbElements = ToBig(0);

	prefix->bTargets += 1;
	DFUPrintf("Update Prefix bTargets: %d\n", prefix->bTargets);

	long image_size = ToBig(prefix->DFUImageSize);
	image_size	+= sizeof(DfuTargetPrefixType);
	prefix->DFUImageSize = ToBig(image_size);
	DFUPrintf("Update Prefix Image Size: %ld\n", image_size);

	return sizeof(DfuTargetPrefixType);
}

/***************************
* add Element
*/
int Dfu_AddElement(DfuPrefixType* prefix, DfuTargetPrefixType* target, DfuElementPrefixType* element, uint32_t addr,const char* buf, uint32_t size){
	DFUPrintf("init target Element...\n");
	DFUPrintf("Element Addr: %08x\n", addr);
	element->dwElementAddress = ToBig(addr);
	DFUPrintf("Element Size: %d\n", size);
	element->dwElementSize = ToBig(size);
	DFUPrintf("Copy Element Data...\n");
	memcpy((uint8_t*)element + sizeof(DfuElementPrefixType), buf, size);
	
	long target_size = ToBig(target->dwTargetSize);
	target_size	+= size + sizeof(DfuElementPrefixType);
	target->dwTargetSize = ToBig(target_size);
	DFUPrintf("Update Target Size: %ld\n", target_size);

	long element_num = ToBig(target->dwNbElements);
	element_num	+= 1;
	target->dwNbElements = ToBig(element_num);
	DFUPrintf("Update Target Elements: %ld\n", element_num);
	
	long image_size = ToBig(prefix->DFUImageSize);
	image_size	+= sizeof(DfuElementPrefixType) + size;
	prefix->DFUImageSize = ToBig(image_size);
	DFUPrintf("Update Prefix Image Size: %ld\n", image_size);

	return sizeof(DfuElementPrefixType) + size;
}

/***************************
* fill Dfu Suffix
*/
int Dfu_AddSuffix(DfuPrefixType* prefix, DfuSuffixType* suffix, int bcd, int pid, int vid){
	DFUPrintf("init Subbfix...\n");
	DFUPrintf("bcd Device: %d\n", bcd);
	suffix->bcdDeviceLo = bcd;
	suffix->bcdDeviceHi = bcd>>8;
	DFUPrintf("Product ID: %d\n", pid);
	suffix->idProductLo = pid;
	suffix->idProductHi = pid>>8;
	DFUPrintf("Vendor ID: %d\n", vid);
	suffix->idVendorLo = vid;
	suffix->idVendorHi = vid>>8;
	DFUPrintf("bcdDFU: %d\n", BCD_DFU);
	suffix->bcdDFULo = (uint8_t)BCD_DFU;
	suffix->bcdDFUHi = BCD_DFU>>8;
	DFUPrintf("Suffix Signature: %s\n", SUFFIX_SIGNATURE);
	memcpy(suffix->ucDfuSignature, SUFFIX_SIGNATURE, sizeof(suffix->ucDfuSignature));
	DFUPrintf("length: %ld\n", sizeof(DfuSuffixType));
	suffix->bLength = sizeof(DfuSuffixType);
	
	uint32_t crc = crc32(0, prefix, (int)(((uint8_t *)suffix-(uint8_t*)prefix) + sizeof(DfuSuffixType) - sizeof(suffix->dwCRC)));
	crc = 0xFFFFFFFF & -crc  - 1 ;
	suffix->dwCRC = ToBig(crc); 
	DFUPrintf("crc: %08x\n", crc);
	return sizeof(DfuSuffixType);
}

