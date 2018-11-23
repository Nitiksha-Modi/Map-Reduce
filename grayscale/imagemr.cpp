
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"
#include <time.h>
#include<iostream>
#include <bits/stdc++.h> 
using namespace std;

//clock_t start,e;
//double time_used;


unsigned char* data;
unsigned char* databw;
int width;
int height;

char* itoa(int val, int base){
	
	static char buf[32] = {0};
	
	int i = 30;
	//if (val==0)
		//return &buf[0];
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
}


void Map(char *file_name) {

	int r,g,b;
	int bw;
	int i=atoi(file_name);
	char *name;
	name=(char *)malloc(20*sizeof(char));
	strcpy(name,file_name);
	
	
	for(int j=0;j<width;j++){
		r=data[3 * (i * width + j)];
		g=data[3 * (i * width + j)+1];
		b=data[3 * (i * width + j)+2];
		bw=(r+g+b)/3;

		strcat(name,"-");
		if(j==0)
			strcat(name,"0");
		else	
			strcat(name,itoa(j,10));
		strcat(name,"-");

		MR_Emit(name,itoa(bw,10));
		

		strcpy(name,file_name);
		
	}
	
}

void Reduce(char *key, Getter get_next, int partition_number) {
	
	
    char *value;
	char *k;
	k=(char *)malloc(20*sizeof(char));
	strcpy(k,key);

	int j;
	char * token;
	char * e;
	token=strtok(k,"-");
	int i=atoi(token);
	e = strchr(token, '-');
	token = strtok(e,"-");
   	
	if(token!=NULL)
		j=atoi(token);
	//else
		//printf("%s\n",key);
	//printf("start reduce %d,%d\n",i,j);
    value = get_next(key, partition_number);
	int a=(data[3 * (i * width + j)+1]+data[3 * (i * width + j)]+data[3 * (i * width + j)+2])/3;
	if (value!=NULL && token!=NULL){
    	databw[3 * (i * width + j)]=atoi(value);
		databw[3 * (i * width + j)+1]=atoi(value);
		databw[3 * (i * width + j)+2]=atoi(value);
	}	
	
	free(k);
}



int main(int argc, char *argv[]) {
	char *filename=argv[1];
	char *filename2=argv[2];

	FILE* f = fopen(filename, "rb");
	assert(f != NULL);

	FILE* f2 = fopen(filename2, "wb");
    
	// read the 54-byte header
	unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); 
	fwrite(info,sizeof(unsigned char),54,f2);
	
    // extract image height and width from header
    width = *(int*)&info[18];
    height = *(int*)&info[22];
	int size = 3*width*height;
	//printf("%d %d\n",width,height);
    data = new unsigned char[size]; // allocate 3 bytes per pixel
	
	databw = new unsigned char[size];

    fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
    fclose(f);

	char **arg=new char*[height+1];
	arg[0]="0";
	arg[1]="0";
	for(int k=2;k<=height;k++){
		//arg[k]=(char *)malloc(sizeof(char)*10);
		//arg[k]=new char[10];
		//char arg[k][10];
		arg[k] = (char *) malloc(100);
		strcpy(arg[k],itoa(k-1,10));
		
		//printf("%s\n",arg[k]);
	}
	
	//printf("%s\n",arg[150]);
	printf("mr run start\n");
	
	/*FILE *fptr1=fopen("grayscale_1024*768.txt","a");
	for(int i=1;i<=60;i++){
		time_used=0;
		for(int j=0;j<20;j++){
	   	start=clock();*/
	   	MR_Run(height+1, arg, Map, atoi(argv[3]), Reduce, atoi(argv[4]), MR_DefaultHashPartition);
		/*e=clock();
	   	time_used+=((double)(e-start))/CLOCKS_PER_SEC;
		printf("%d\n",i);
		}
		fprintf(fptr1,"%f\n",time_used/20);
	}
	fclose(fptr1);*/

	printf("mr run end\n");

	fwrite(databw,sizeof(unsigned char),size,f2);
	fclose(f2);
	
}
