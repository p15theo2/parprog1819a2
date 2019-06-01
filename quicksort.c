//diafora includes
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define THREADS 4 //Ammount of threads
#define Table_size 100 //Size of table , to be sorted
#define queue_size 1000000 //Size of messages queue
#define THRESHOLD 10//When the program will use inshort ?

#define WORK 0
#define FINISH 1
#define TERMINATE 2

struct message//message struct 
{
    int type;
    int start;
    int end;
};

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;
pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;

struct message mQueue[queue_size];
int qin = 0,qout = 0,msg_count = 0;//pointers and counters



void swap(double *el1, double *el2) //function gia swap metablhtvn
{
  double tmp = *el1;
  *el1 = *el2;
  *el2 = tmp;
} 


int partition(double *a,int end) {//function gia thn taksinomisi ton arithon
    
    int first = 0;
    int middle = end/2;
    int last = end-1;

    if (a[first] > a[middle]) 
    {
        swap(a+first, a+middle);
    }
    if (a[middle] > a[last]) 
    {
        swap(a+middle, a+last);
    }
    if (a[first] > a[middle])
    {
        swap(a+first, a+middle);
    }

    double pivot = a[middle];
    int i, j;
    for (i=1, j=end-2;; i++, j--) 
    {
        while (a[i] < pivot) i++;
        
        while (a[j] > pivot) j--;
        
        if (i>=j)
        break;

        swap(a+i, a+j);
    }
    return i;
}

void send(int type,int start,int end)//apostolh mynhmaton
{
  pthread_mutex_lock(&mutex);

  while(msg_count >= queue_size)
  {
  printf(" PRODUCER LOCKED \n");
  pthread_cond_wait(&msg_out, &mutex);
  }

  mQueue[qin].type = type;
  mQueue[qin].start = start;
  mQueue[qin].end = end;
  qin = (qin + 1) % queue_size;

  msg_count++;


  pthread_cond_signal(&msg_in);
  pthread_mutex_unlock(&mutex);

}


void quicksort(double *a,int begin,int end){//algorithmos quickshort
int split = partition(a+begin,end-begin) ;   
send(WORK,begin,begin+split);
send(WORK,begin+split,end);   
}


void insort(double *a,int end){//algorithmos inshort
    int i,j;
    for (i=1;i<end;i++){
        j=i;
        while ( j>0 && a[j-1]>a[j] )
        {
              swap(a+j-1,a+j);
            j--;
        }
    }
}


void receive(int *type,int *start,int *end)//paralahpsi mynhmaton
{
  pthread_mutex_lock(&mutex);

  while(msg_count<1)
  {
  printf(" Consumer locked \n");
  pthread_cond_wait(&msg_in,&mutex);
  }
  *type = mQueue[qout].type;
  *start = mQueue[qout].start;
  *end = mQueue[qout].end;
  qout = (qout + 1) % queue_size;
  msg_count--;

  pthread_cond_signal(&msg_out);
  pthread_mutex_unlock(&mutex);

}

void *thread_func(void *params){//o kodikas pou trexei kathe thread

    double *a = (double*) params;
    
    int typ,begin,end;

    receive(&typ,&begin,&end);

    while (typ !=TERMINATE){
        
      if(typ == FINISH){
            printf("Finish\n");
            send(FINISH,begin,end);
        
       }else if (typ == WORK)
            {
            if ((end-begin)<=THRESHOLD)
            {
              insort(a+begin,end-begin);
              send(FINISH,begin,end);
            }
            else
            {            
            quicksort(a,begin,end);
            }
        }
        printf("type  %d,begin %d,end %d\n",typ,begin,end);
        receive(&typ,&begin,&end);
    }
    printf("Shutdown\n");
    send(TERMINATE,0,0);
    pthread_exit(NULL);
}




int main() {//main 

    double *a=(double*) malloc(sizeof(double) * Table_size);
    pthread_t thrending[THREADS];
    printf("Allocating a\n");

    if (a == NULL) {
        printf("memory allocation error\n");
        exit(-1);
    }

    

    int i;
    for(i=0;i<Table_size;i++)
    {
        a[i] = (double)rand()/RAND_MAX;
    }

    
    for(int i=0;i<THREADS;i++)
    {
        pthread_create(&thrending[i],NULL,&thread_func,a);
    }


    send(WORK,0,Table_size);

    int typ,begin,end;
    int completed=0;

    receive(&typ,&begin,&end);

    while (1){

        if (typ == FINISH)
        {
            completed += end - begin;
            printf("completed %d\n",completed);
        if (completed == Table_size) {
        break;}
        }
        else 
        {
            send(typ,begin,end);
        }
        receive(&typ,&begin,&end);
    }
 
    send(TERMINATE,0,0); 
 
    for (int i = 0 ;i<Table_size-1;i++ )
    {
        if (a[i]>a[i+1])
        {
         printf("Error table not sorted");
         break;
        }
     }
    
    
     printf("Destroying threads\n");

     for(int i=0;i<THREADS;i++)
     {
        pthread_join(thrending[i],NULL);
     }
     pthread_mutex_destroy(&mutex);
     pthread_cond_destroy(&msg_in);
     pthread_cond_destroy(&msg_out);
     free(a);


    return 0;
}










