#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "dfuse.h"

#define ENTRY(t,x,n)		((t)(((uint8_t*)x)+n))
char*  ReadFile(const char* path, size_t *len)
{
	int ret;
	FILE *fp;
	struct stat stat_buf;
	char* data;

	ret = stat(path, &stat_buf);
	if(ret < 0){	
		perror("stat:");
		return NULL;
	}
	
	if(stat_buf.st_size < 1){
		printf("incorrect file size!\n");
		return NULL;
	}

	fp = fopen(path, "r");
	if(fp == NULL){
		perror("open:");
		return NULL;
	}
	
	data = malloc(stat_buf.st_size);	
	if(data == NULL){
		perror("malloc:");
		fclose(fp);
		return NULL;
	}

	ret = fread(data, 1, stat_buf.st_size, fp);
	if(ret != stat_buf.st_size){
		printf("Warning: Unable read out all data!\n");
	}
	
	if(len)
		*len = ret;

	return data;
}

cJSON* ParseJson(char* path)
{
	char* buf = ReadFile(path, NULL);
	if(buf!=NULL){
    	cJSON* root = cJSON_Parse(buf);
		free(buf);
		return root;
	}
	return NULL;
}

int GetIntValue(cJSON* root, char* key, int preset){
	cJSON *item = cJSON_GetObjectItem(root, key);
	if(item){
		return item->valueint;
	}else{
		printf("Warning: Unable found key [%s]. set to default [%d]\n", key, preset);
		return preset;
	}
}

const char* GetStringValue(cJSON* root, char* key, char* preset){
	cJSON *item = cJSON_GetObjectItem(root, key);
	if(item){
		return item->valuestring;
	}else{
		if(preset)
			printf("Warning: Unable found key [%s]. set to default [%s]\n", key, preset);
		return preset;
	}
}

uint32_t  GetHexValue(cJSON* root, char* key, uint32_t preset){
	cJSON *item = cJSON_GetObjectItem(root, key);
	if(item){
		return strtol(item->valuestring, NULL, 16);
	}else{
		if(preset)
			printf("Warning: Unable found key [%s]. set to default [%08x]\n", key, preset);
		return preset;
	}
}

int main(int argc, char* argv[]){
	int offset = 0;
	char* buf;

	/*parse layout file*/
	cJSON *layout = ParseJson("stm32f411x.json");
	if(layout == NULL){
		printf("unable prase layout file\n");
		return -1;
	}

	/*get max file length*/	
	int size  = GetIntValue(layout, "MaxSize", 2*1024*1024);
	buf = malloc(size);
	if(buf == NULL)
	{
		perror("malloc:");
		cJSON_Delete(layout);
		return -1;
	}

	/*start gen dfu image*/
	DfuPrefixType *prefix = ENTRY(DfuPrefixType*, buf, offset);
	/*add profix*/
	offset += Dfu_AddPrefix(prefix,1); 

	/*start target*/
	cJSON *targetArray = cJSON_GetObjectItem(layout, "target");
	if(targetArray == NULL){
		printf("no target found!\n");
		cJSON_Delete(layout);
		free(buf);
		return -1;
	}		

	for(int i = 0; i < cJSON_GetArraySize(targetArray); i++){
		cJSON* targetLayout = cJSON_GetArrayItem(targetArray, i);	
		const char* targetName = GetStringValue(targetLayout, "name", NULL);
		int alt = GetIntValue(targetLayout, "alternatesetting", 0);
		/*gen target prefix*/
		DfuTargetPrefixType *target = ENTRY(DfuTargetPrefixType*, buf, offset);
		offset += Dfu_AddImage(prefix, target, alt, targetName);
	
		/*start Element*/
		cJSON *elementArray = cJSON_GetObjectItem(targetLayout, "element");
		if(elementArray == NULL){
			printf("Empty element target. continue...\n");
			continue;
		}		
	
		for(int j = 0; j <cJSON_GetArraySize(elementArray); j++){
			cJSON* elementLayout = cJSON_GetArrayItem(elementArray, j);	
			//const char* elementName =  GetStringValue(elementLayout, "name", NULL);
			const char* elementSource =  GetStringValue(elementLayout, "source", NULL);
			uint32_t elementAddr = GetHexValue(elementLayout, "address", -1);
			uint32_t elementSize = GetHexValue(elementLayout, "size", -1);
			int elementPresent = GetIntValue(elementLayout, "essential", 0);
			/*read image file*/
			size_t len;
			char* rawdata = ReadFile(elementSource, &len);
			if(rawdata == NULL){
				if(elementPresent == 1){
					printf("Unalbe Read Image file:%s Exit...", elementSource);
					cJSON_Delete(layout);
					free(buf);
					return -1;
				}else{
					printf("Warning: Unable Read Image file:%s continue...\n", elementSource);
					continue;
				}
			}
			
			if(len > elementSize){
				printf("Image file size outof range:%s\n",elementSource);
				cJSON_Delete(layout);
				free(buf);
				return -1;
			}
	
			DfuElementPrefixType* element = ENTRY(DfuElementPrefixType*, buf, offset);
			offset += Dfu_AddElement(prefix, target, element, elementAddr, rawdata, len);
			free(rawdata);
		}
	}	

	uint32_t bcd = GetHexValue(layout, "bcdDevice", 0xFFFF);
	uint32_t pid = GetHexValue(layout, "PID", 0xFFFF);
	uint32_t vid = GetHexValue(layout, "VID", 0xFFFF);
	DfuSuffixType* suffix = ENTRY(DfuSuffixType*, buf, offset);
	offset +=Dfu_AddSuffix(prefix, suffix, bcd,  pid, vid);
	
	FILE *out = fopen("out.dfu", "w");
	if(!out){
		printf("Unable write to output file!\n");
	}else
		fwrite(buf, 1, offset, out);
	fclose(out);
	cJSON_Delete(layout);	
	free(buf);
	return 0;
}
