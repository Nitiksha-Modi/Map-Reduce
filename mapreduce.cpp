#include "mapreduce.h"	//headers
#include <thread>   		//threads
#include <mutex>    		//locks
#include <string.h> 		//strcmp,strcpy
#define NUM_MAPS 800

int NUM_FILES;
Mapper UMAP;
int NUM_MAPPERS;
Reducer UREDUCE;
int NUM_REDUCERS;
Partitioner UPARTITION;
int counter;
std::mutex mainlock;
char** file_names;

class v_node{
	public:
		char* value;
		v_node* next;
};

class k_node{
	public:
		char* key;
		v_node* head;
		k_node* next;
};

class k_entry{
	public:
		k_node* head;
		std::mutex lock;
};

class p_entry{
	public:
		k_entry map[NUM_MAPS];
		int key_num;
		std::mutex lock;
		k_node* srt;
		int curr_visit;
};

p_entry hmap[64];



//additional functions..............................................................

//string comparision for sorting

int comp_strings(const void* a, const void* b){
    char* n1 = ((k_node*)a)->key;
    char* n2 = ((k_node*)b)->key;
    return strcmp(n1, n2);
    
}

// function to get values for a given key and partition

char* get_func(char *key, int p){		//p is partion number
    k_node* A = hmap[p].srt;
    char* value;
	int c = hmap[p].curr_visit;
    while(1)
	{
        if (strcmp(A[c].key, key) == 0)
		{
            if (A[c].head == NULL)
                return NULL;
			else{						
            	v_node* temp = A[c].head->next;
            	value = A[c].head->value;
            	A[c].head = temp;
            	return value;
			}
        } 
		else {
            c++;
			continue;
        }
        return NULL;
    }
}


//function for sorting keys to be given to the reduce function.

void sortedkeys(int p){

	hmap[p].srt =(k_node *) malloc(hmap[p].key_num*sizeof(k_node));

    int count = 0;
    for (int i = 0; i < NUM_MAPS; i++){
        k_node *c = hmap[p].map[i].head;
        
        while (c != NULL){
            hmap[p].srt[count] = *c;
            count++;
            c = c->next;
        }
    }
    qsort(hmap[p].srt, hmap[p].key_num, sizeof(k_node), comp_strings);
}
//.................................................................................................................


//default hash partition
unsigned long MR_DefaultHashPartition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
	//printf("%d partitions\n",hash % num_partitions);
    return hash % num_partitions;
}



//function for map for each thread
void ThreadMap(){
    while(1){
        char* fname;
        mainlock.lock();
        if(counter >= NUM_FILES){
            mainlock.unlock();
            return;
        }
		else{
        	fname = file_names[counter++];
        	mainlock.unlock();
        	(*UMAP)(fname);
		}
    }
}

//function for reduce for each thread
void ThreadReduce(int p){		//p is partition number
    
    if(hmap[p].key_num == 0)
        return;
    sortedkeys(p);
   
    for (int i = 0; i < hmap[p].key_num; i++){
        (*UREDUCE)(hmap[p].srt[i].key,get_func,p);
    }
	
	//free memory
    for (int i = 0; i < NUM_MAPS; i++){
        k_node *c = hmap[p].map[i].head;
        while (c != NULL){
			free(c->key);
            c->key = NULL;
            v_node* v = c->head;
            while (v!=NULL){
                v->value = NULL;
                v_node* temp = v->next;
				free(v);
                v=temp;
            }
            v=NULL;
            k_node* temp2 = c->next;
			free(c);
            c = temp2;
        }
        c = NULL;
    }
    free(hmap[p].srt);
    hmap[p].srt = NULL;

    return;
}


// MR_Emit function
void MR_Emit(char *key, char *value){
    
    unsigned long p = (*UPARTITION)(key, NUM_REDUCERS);
    unsigned long m = MR_DefaultHashPartition(key, NUM_MAPS);
    hmap[p].map[m].lock.lock();
    k_node* temp = hmap[p].map[m].head;
    while(temp != NULL){
        if (strcmp(temp->key, key) == 0)
            break;
        temp = temp->next;
    }
    
    //create a value node

    v_node* new_v = (v_node *)malloc(sizeof(v_node));
    if (new_v == NULL) {
        perror("malloc");
        hmap[p].map[m].lock.unlock();
        return; // fail
    } 
    new_v->value = (char*)malloc(sizeof(char)*20);
    if (new_v->value == NULL)
        printf("ERROR MALLOC FOR VALUE");
    strcpy(new_v->value, value);
    new_v->next = NULL;
    //if there is no existing node for same key
    if (temp == NULL){
        k_node *new_key =(k_node *) malloc(sizeof(k_node));
        if (new_key == NULL) {
            perror("malloc");
            hmap[p].map[m].lock.unlock();
            return; // fail
        }
        new_key->head = new_v;
        new_key->next = hmap[p].map[m].head;
        hmap[p].map[m].head = new_key;
        
        new_key->key =(char *) malloc(sizeof(char)*20);
        if (new_key->key == NULL)
            printf("ERROR MALLOC FOR VALUE");
        strcpy(new_key->key, key);
        hmap[p].lock.lock();
        hmap[p].key_num++;
        hmap[p].lock.unlock();

    } else {
        //if there is existing node for same key
        new_v->next = temp->head;
        temp->head = new_v;
    }

    hmap[p].map[m].lock.unlock();
}



//MR_Run function
void MR_Run(int argc, char *argv[], 
        Mapper map, int num_mappers, 
        Reducer reduce, int num_reducers, 
        Partitioner partition){

    UPARTITION = partition;
    NUM_REDUCERS = num_reducers;
    NUM_MAPPERS = num_mappers;
    NUM_FILES = argc - 1;
    file_names = &argv[1];
    UMAP = map;
    UREDUCE = reduce;
    counter = 0;

    for (int i = 0;i < NUM_REDUCERS; i++){
        hmap[i].key_num = 0;
        hmap[i].curr_visit = 0;
        hmap[i].srt = NULL;
        for (int j = 0; j < NUM_MAPS; j++){
            hmap[i].map[j].head = NULL;
        } 
    }

    // create map threads
    std::thread mt[NUM_MAPPERS];
    for (int i = 0; i < NUM_MAPPERS; i++){
        mt[i] = std::thread(ThreadMap);
    }

    // join waits for the threads to finish
    for (int k = 0; k < num_mappers; k++){
        mt[k].join();
    }

    // create reduce threads
    std::thread rt[NUM_REDUCERS];
    for (int j = 0; j < NUM_REDUCERS; j++){
        rt[j] = std::thread(ThreadReduce, j);
    }
	// join waits for the threads to finish
    for (int m = 0; m < NUM_REDUCERS; m++){
        rt[m].join();
    }
}
