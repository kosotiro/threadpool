#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>




void *primetest(void *n){


   int k = *((int *)n);

   int i,counter=0;

   //printf("primetest:%d\n",k);

   for(i=1;i<=k;i++)
   {
     if(k%i==0)
       counter++;

   }   

   return counter>=3?(void *)0:(void *)1;

}


void *dothreadWork(void *args){


thpool *thp = (thpool *)args;


funcptr f;

void *arg;

//printf("Mphka!:%d\n",thp->jobquesize);



while(1){

    pthread_mutex_lock(&(thp->mtx));

    

    while(thp->occupiedjobs==0){

          if(thp->destruction_started)
          {
            pthread_mutex_unlock(&(thp->mtx)); 
            pthread_exit(NULL);
            return NULL;
          }

          pthread_cond_wait(&(thp->q_notempty),&(thp->mtx));


          if(thp->destruction_started)
          {
            pthread_mutex_unlock(&(thp->mtx)); 
            pthread_exit(NULL);
            return NULL;
          }
    }



    f = thp->jobqueue[thp->jobhead].f;
    arg = thp->jobqueue[thp->jobhead].args;


    //printf("jobqueuePULL:%p\n",arg);

    --(thp->occupiedjobs);

    thp->jobhead = (thp->jobhead+1)%thp->jobquesize;

    if(thp->occupiedjobs==0 && !thp->destruction_started)
      pthread_cond_broadcast(&(thp->q_empty));
 

    pthread_mutex_unlock(&(thp->mtx));

    

    printf("gia to arg:%d To apotelesma einai:%ld\n",*((int *)arg), (long )f(arg));

    

}



}


thpool *createWorkers(int numofthreads,int nofjobs){

int i;



thpool *thp = (thpool *)malloc(sizeof(thpool));

//printf("Kostas0:%p\n",thp);


if(thp==NULL)
{
  return NULL;
}


thp->jobqueue = (job *) malloc(sizeof(job) *nofjobs);



if(thp->jobqueue == NULL)
{
   //printf("Wx mphka edw pera!\n");
   free(thp);
   return NULL;
}

//printf("Kostas2");

thp->threadqueue = (pthread_t *)malloc(sizeof(pthread_t)*numofthreads);

if(thp->threadqueue == NULL)
{
  free(thp->jobqueue);
  free(thp);
  return NULL;
}

//printf("Kostas3");
pthread_mutex_init(&thp->mtx,NULL);

if(pthread_cond_init(&thp->q_notempty,NULL))
  return NULL;

pthread_cond_init(&thp->q_empty,NULL);

//printf("Kostas4");


thp->jobquesize = nofjobs;
thp->jobtail = thp->jobhead = 0;
thp->occupiedjobs = 0;
thp->threadsOccupied = 0;
thp->nofthreads = numofthreads;
thp->destruction_started = false;

//printf("Kostas");


for(i=0;i<numofthreads;i++)
{
  if(pthread_create(&(thp->threadqueue[i]),NULL,dothreadWork,thp)>0)
  {
    return NULL;
  }

}

return thp;

}





void jobqueuepush(thpool **thp,funcptr f,void *args){


   pthread_mutex_lock(&((*thp)->mtx));

   if((*thp)->jobquesize == (*thp)->occupiedjobs || (*thp)->destruction_started)
   {
     pthread_mutex_unlock(&((*thp)->mtx));
     return;
   }


   //printf("pointer:%p\n",*thp);
   ((*thp)->jobqueue[(*thp)->jobtail]).f = f;
   ((*thp)->jobqueue[(*thp)->jobtail]).args = args;


   //printf("jobqueuepush:%p\n",args);
     
   ++(*thp)->occupiedjobs;

   (*thp)->jobtail = ((*thp)->jobtail + 1)%((*thp)->jobquesize);


   if((*thp)->occupiedjobs==1)
     pthread_cond_signal(&((*thp)->q_notempty));


   pthread_mutex_unlock(&((*thp)->mtx));


}


bool thpooldestroy(thpool **thp){

int i;

pthread_mutex_lock(&((*thp)->mtx));

   if((*thp)->destruction_started) //in case that thpooldestroy has been called again
   {
     pthread_mutex_unlock(&((*thp)->mtx));
     return true;
   }

   while((*thp)->occupiedjobs>0)
   {
     pthread_cond_wait(&((*thp)->q_empty),&((*thp)->mtx));
     
   }
   (*thp)->destruction_started = true;
  

pthread_mutex_unlock(&((*thp)->mtx));

    pthread_cond_broadcast(&((*thp)->q_notempty));  //allow code to return NULL;

   for(i=0;i<(*thp)->nofthreads;i++)
       pthread_join((*thp)->threadqueue[i],NULL);


   (*thp)->nofthreads = 0;

   //kane free ta resources
   free((*thp)->jobqueue);
   (*thp)->jobqueue = NULL;

   free((*thp)->threadqueue);
   (*thp)->threadqueue = NULL;
   

   free(*thp);

return true;

}



int main(int argc,void *argv[]){

   int i;

   srand(time(NULL));

   thpool *p = createWorkers(4,1000000);

   int a[1000000];

   for(i=0;i<1000000;i++)
      a[i] = (rand()%100)+1;
   
   

   if(p!=NULL)
   {

     for(i=1;i<1000000;i++)
       jobqueuepush(&p,(funcptr)primetest,(void *)&a[i]);
       
   }

   thpooldestroy(&p);

   return(EXIT_SUCCESS);
}
