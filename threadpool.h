#include <pthread.h>
#include <stdbool.h>


typedef void *(*funcptr)(void *);

typedef enum JOBSTATUS{
        COMPLETED,
        RUNNING,
        PENDING
}jobstatus;

typedef struct _job{
 funcptr f;
 void *args;
} job;


typedef struct _thpool{

job *jobqueue;
int jobquesize;
int jobtail;
int jobhead;
int occupiedjobs;
bool destruction_started;

pthread_t *threadqueue;
int nofthreads;
int threadsOccupied;
pthread_mutex_t mtx;
pthread_cond_t q_notempty;
pthread_cond_t q_empty;

}thpool;


thpool *createWorkers(int numofthreads,int nofjobs);
