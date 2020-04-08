#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
pthread_mutex_t lock_resource;
pthread_cond_t deadlock_condition;
int no_of_resource,no_of_process;
int *resource;
int **allocated_matrix;
int **maximum_required;
int **need_matrix;
int *safe_sequence;
int no_of_processRan = 0;
// get safe sequence if there is one else return false
bool getsafe_sequence();

void* processCode(void* );

int main(int argt, char** argq) {
	srand(time(NULL));
        printf("\n Enter Number of resources ");
        scanf("%d", &no_of_resource);
        printf("\n enter number of processes you want to check ");
        scanf("%d", &no_of_process);
        resource = (int *)malloc(no_of_resource * sizeof(*resource));
        printf("\n Currently Available resources (R1 R2 ...) ");
        for(int i=0; i<no_of_resource; i++)
                scanf("%d", &resource[i]);
        allocated_matrix = (int **)malloc(no_of_process * sizeof(*allocated_matrix));
        for(int i=0; i<no_of_process; i++)
                allocated_matrix[i] = (int *)malloc(no_of_resource * sizeof(**allocated_matrix));
        maximum_required = (int **)malloc(no_of_process * sizeof(*maximum_required));
        for(int i=0; i<no_of_process; i++)
                maximum_required[i] = (int *)malloc(no_of_resource * sizeof(**maximum_required));
        // allocated
        printf("\n");
        for(int i=0; i<no_of_process; i++) {
                printf("\n Allocate resources to the process %d  (R1 R2 ...) ", i+1);
                for(int j=0; j<no_of_resource; j++)
                        scanf("%d", &allocated_matrix[i][j]);
        }
        printf("\n");
	// maximum number of  required resources by a particular process
        for(int i=0; i<no_of_process; i++) {
                printf("\n Enter the maximum resources required by process %d (R1 R2 ...) ", i+1);
                for(int j=0; j<no_of_resource; j++)
                        scanf("%d", &maximum_required[i][j]);
        }
        printf("\n");
	//we are calculating need matrix
        need_matrix = (int **)malloc(no_of_process * sizeof(*need_matrix));
        for(int i=0; i<no_of_process; i++)
                need_matrix[i] = (int *)malloc(no_of_resource * sizeof(**need_matrix));
        for(int i=0; i<no_of_process; i++)
                for(int j=0; j<no_of_resource; j++)
                        need_matrix[i][j] = maximum_required[i][j] - allocated_matrix[i][j];
	// get safe sequence to prevent deadlock in future
	safe_sequence = (int *)malloc(no_of_process* sizeof(*safe_sequence));
        for(int i=0; i<no_of_process; i++) safe_sequence[i] = -1;
        if(!getsafe_sequence()) {
                printf("\n Unsafe State!Dealock may occur. The processes and resources leads the system to a unsafe state.\n\n");
                exit(-1);
        }
        printf("\n\n Safe Sequence Found : ");
        for(int i=0; i<no_of_process; i++) {
                printf("%d", safe_sequence[i]+1);
        }
        printf("\n Process are getting executed...\n\n");
        sleep(1);
	// run threads
	pthread_t processes[no_of_process];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
	int processNumber[no_of_process];
	for(int i=0; i<no_of_process; i++) processNumber[i] = i;
        for(int i=0; i<no_of_process; i++)
                pthread_create(&processes[i], &attr, processCode, (void *)(&processNumber[i]));
        for(int i=0; i<no_of_process; i++)
                pthread_join(processes[i], NULL);
        printf("\n All Processes execution is Finished\n");	
	//checking free resources
        free(resource);
        for(int i=0; i<no_of_process; i++) {
                free(allocated_matrix[i]);
                free(maximum_required[i]);
		free(need_matrix[i]);
        }
        free(allocated_matrix);
        free(maximum_required);
	free(need_matrix);
        free(safe_sequence);
}
bool getsafe_sequence() {
	// get safe sequence
        int tempRes[no_of_resource];
        for(int i=0; i<no_of_resource; i++) tempRes[i] = resource[i];
        bool finished[no_of_process];
        for(int i=0; i<no_of_process; i++) finished[i] = false;
        int nfinished=0;
        while(nfinished <no_of_process) {
                bool safe = false;
                for(int i=0; i<no_of_process; i++) {
                        if(!finished[i]) {
                                bool possible = true;
                                for(int j=0; j<no_of_resource; j++)
                                        if(need_matrix[i][j] > tempRes[j]) {
                                                possible = false;
                                                break;
                                        }
                                if(possible) {
                                        for(int j=0; j<no_of_resource; j++)
                                                tempRes[j] += allocated_matrix[i][j];
                                        safe_sequence[nfinished] = i;
                                        finished[i] = true;
                                        ++nfinished;
                                        safe = true;
                                }
                        }
                }
                if(!safe) {
                        for(int k=0; k<no_of_process; k++) safe_sequence[k] = -1;
                        return false; // no safe sequence found
                }
        }
        return true; // safe sequence found
}
// process code
void* processCode(void *arg) {
        int p = *((int *) arg);
	// locking the resources
        pthread_mutex_lock(&lock_resource);
        // condition check
        while(p != safe_sequence[no_of_processRan])
                pthread_cond_wait(&deadlock_condition, &lock_resource);
	// process
        printf("\n--> Process %d", p+1);
        printf("\n\tAllocated : ");
        for(int i=0; i<no_of_resource; i++)
                printf("%d", allocated_matrix[p][i]);
        printf("\n\tNeeded    : ");
        for(int i=0; i<no_of_resource; i++)
                printf("%d", need_matrix[p][i]);
        printf("\n\tAvailable : ");
        for(int i=0; i<no_of_resource; i++)
                printf("%d", resource[i]);
        printf("\n"); sleep(1);
        printf("\t Resources Allocated!");
        printf("\n"); sleep(1);
        printf("\t Process Code Running...");
        printf("\n"); sleep(rand()%3 + 2); // process code
        printf("\t Process Code Completed...");
        printf("\n"); sleep(1);
        printf("\t Process is Releasing Resource...");
        printf("\n"); sleep(1);
        printf("\t Resource Released!");
	for(int i=0; i<no_of_resource; i++)
                resource[i] += allocated_matrix[p][i];
	printf("\n\t Now Available resouces are: ");
        for(int i=0; i<no_of_resource; i++)
                printf("%3d", resource[i]);
        printf("\n\n");
        sleep(1);
	// condition broadcast
        no_of_processRan++;
        pthread_cond_broadcast(&deadlock_condition);
        pthread_mutex_unlock(&lock_resource);
	pthread_exit(NULL);
}
