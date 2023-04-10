#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int noOfresources,noOfprocesses;
int *resources;
int **allocated;
int **maxrequired;
int **need;
int *safeSequence;
int noOfprocesscompleted = 0;

pthread_mutex_t lockresources;
pthread_cond_t condition;
bool getSafeSequence();
void* process(void* c);

int main(int a, char** b) 
{
	    srand(time(NULL));

        printf("\nNumber of processes : ");
        scanf("%d", &noOfprocesses);

        printf("\nNumber of resources (A B C ...) : ");
        scanf("%d", &noOfresources);

        resources = (int *)malloc(noOfresources * sizeof(*resources));
        printf("\nCurrently Available resources's instance(s) after allocation : ");
        for(int i=0; i<noOfresources; i++)
                scanf("%d", &resources[i]);

        allocated = (int **)malloc(noOfprocesses * sizeof(*allocated));
        for(int i=0; i<noOfprocesses; i++)
                allocated[i] = (int *)malloc(noOfresources * sizeof(*allocated));

        maxrequired = (int **)malloc(noOfprocesses * sizeof(*maxrequired));
        for(int i=0; i<noOfprocesses; i++)
                maxrequired[i] = (int *)malloc(noOfresources * sizeof(*maxrequired));

        printf("\n");
        for(int i=0; i<noOfprocesses; i++) {
                printf("\nResource's instance allocated to process %d (A B C ...) : ", i+1);
                for(int j=0; j<noOfresources; j++)
                        scanf("%d", &allocated[i][j]);
        }
        printf("\n");
        

        for(int i=0; i<noOfprocesses; i++) {
                printf("\nMaximum resource's instance required by process %d (A B C ...) : ", i+1);
                for(int j=0; j<noOfresources; j++)
                        scanf("%d", &maxrequired[i][j]);
        }
        printf("\n");

        need = (int **)malloc(noOfprocesses * sizeof(*need));
        for(int i=0; i<noOfprocesses; i++)
                need[i] = (int *)malloc(noOfresources * sizeof(*need));

        for(int i=0; i<noOfprocesses; i++)
                for(int j=0; j<noOfresources; j++)
                        need[i][j] = maxrequired[i][j] - allocated[i][j];

	safeSequence = (int *)malloc(noOfprocesses * sizeof(*safeSequence));
        for(int i=0; i<noOfprocesses; i++) safeSequence[i] = -1;

        if(!getSafeSequence()) {
                printf("\nThis is unsafe state may result to deadlock.\n\n");
                exit(-1);
        }
        printf("\nGenerating safe sequence\n");
        sleep(1);
        
        printf("\nSafe Sequence Found : \n");
        for(int i=0; i<noOfprocesses; i++) {
                printf("%-3d", safeSequence[i]+1);
        }

        
	
	pthread_t processes[noOfprocesses];
        pthread_attr_t attr;
        pthread_attr_init(&attr);

	int processNumber[noOfprocesses];
	for(int i=0; i<noOfprocesses; i++) processNumber[i] = i;

        for(int i=0; i<noOfprocesses; i++)
                pthread_create(&processes[i], &attr, process, (void *)(&processNumber[i]));

        for(int i=0; i<noOfprocesses; i++)
                pthread_join(processes[i], NULL);

        printf("\nAll Processes Finished\n");	
	
        free(resources);
        for(int i=0; i<noOfprocesses; i++) {
                free(allocated[i]);
                free(maxrequired[i]);
		free(need[i]);
        }
        free(allocated);
        free(maxrequired);
	    free(need);
        free(safeSequence);
}


bool getSafeSequence() {
        int tempRes[noOfresources];
        for(int i=0; i<noOfresources; i++) tempRes[i] = resources[i];

        bool finished[noOfprocesses];
        for(int i=0; i<noOfprocesses; i++) finished[i] = false;
        int noOffinished=0;
        while(noOffinished < noOfprocesses) {
                bool safe = false;

                for(int i=0; i<noOfprocesses; i++) {
                        if(!finished[i]) {
                                bool possible = true;

                                for(int j=0; j<noOfresources; j++)
                                        if(need[i][j] > tempRes[j]) {
                                                possible = false;
                                                break;
                                        }

                                if(possible) {
                                        for(int j=0; j<noOfresources; j++)
                                                tempRes[j] += allocated[i][j];
                                        safeSequence[noOffinished] = i;
                                        finished[i] = true;
                                        ++noOffinished;
                                        safe = true;
                                }
                        }
                }

                if(!safe) {
                        for(int k=0; k<noOfprocesses; k++) safeSequence[k] = -1;
                        return false; 
                }
        }
        return true; 
}


void* process(void *arg) {
        int p = *((int *) arg);

        pthread_mutex_lock(&lockresources);

        while(p != safeSequence[noOfprocesscompleted])
                pthread_cond_wait(&condition, &lockresources);

        printf("\nProcess %d", p+1);
        printf("\nAllocated resources : ");
        for(int i=0; i<noOfresources; i++)
                printf("%3d", allocated[p][i]);

        printf("\nNeeded resources    :  ");
        for(int i=0; i<noOfresources; i++)
                if (need[p][i] < 0){
                    printf(" 0 ");
                }
                else{
                printf("%3d", need[p][i]);
                }
        printf("\nAvailable resources : ");
        for(int i=0; i<noOfresources; i++)
                printf("%3d", resources[i]);
        sleep(rand()%3 + 2);

	for(int i=0; i<noOfresources; i++)
                resources[i] += allocated[p][i];

        printf("\n\tNow Available after completion of process  : ");
        for(int i=0; i<noOfresources; i++)
                printf("%3d", resources[i]);
        printf("\n\n");

        sleep(1);

        noOfprocesscompleted++;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&lockresources);
	pthread_exit(NULL);
}