#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_NUM 1000000
int main(int argc, char *argv[]){
	FILE *handle;
	handle = fopen(argv[1],"w");
	int i;
	for (i = 0; i < MAX_NUM; i++)
		fprintf(handle,"%d ",i);
	fprintf(handle,"\n");
	fclose(handle);
	return 0;
} 
