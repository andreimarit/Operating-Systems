#include "a2_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

typedef struct{
    int nrP;
    int nrT;
} th_str;

int sem_id;
int running_threads=0;

void P(int sem_id, int semNumber) {
    struct sembuf op = {semNumber, -1, 0};
    semop(sem_id, &op, 1);
}

void V(int sem_id,int semNumber) {
    struct sembuf op= {semNumber, +1, 0};
    semop(sem_id,&op,1);
}


void* thread_function7(void* arg)
{
    th_str* th=(th_str*)arg;

    //int p_number=th->nrP;
    int t_number=th->nrT;
    if(t_number==1) P(sem_id,2);

    if (t_number==5)  P(sem_id, 0);
    info(BEGIN, 7, t_number);
    if(t_number==4) {
        V(sem_id, 0);
        P(sem_id, 1);
    }
    info(END, 7, t_number);
    if(t_number==1) V(sem_id, 3);
    if (t_number==5)  V(sem_id, 1);
    return NULL;
}

void* thread_function5(void* arg)
{
    th_str* th=(th_str*)arg;

    //int p_number=th->nrP;
    int t_number=th->nrT;
    if (t_number == 2) P(sem_id, 3);
     
    info(BEGIN, 5, t_number);
    info(END, 5, t_number);
    if(t_number==5) V(sem_id, 2);
    
    return NULL;
}


void* thread_function8(void* arg)
{
    th_str* th=(th_str*)arg;
    //int p_number=th->nrP;
    int t_number=th->nrT;
    
    P(sem_id, 5); 
    info(BEGIN, 8, t_number);
    
    if(t_number==15) P(sem_id, 4);
    if(semctl(sem_id, 5, GETVAL)==0) V(sem_id, 4);
    
    info(END, 8, t_number);
    V(sem_id, 5);
    
    return NULL;
}

int p1, p2, p4, p5, p6, p7, p8;

int main(){
    //printf("Aa");
    init();
    
    sem_id = semget(IPC_PRIVATE, 6, IPC_CREAT | 0600);
    if(sem_id<0) {
        perror("ERROR set semaphore");
        exit(2);
    }  
    //same P
    semctl(sem_id, 0, SETVAL, 0);
    semctl(sem_id, 1, SETVAL, 0);
    //diff P
    semctl(sem_id, 2, SETVAL, 0);
    semctl(sem_id, 3, SETVAL, 0);
    //Barrier
    semctl(sem_id, 4, SETVAL, 0);
    semctl(sem_id, 5, SETVAL, 6);
   
    //Process 1
    info(BEGIN, 1, 0);
    int p1=fork();
    switch (p1)
    {
       case -1: 
            printf("error");
            printf("error -1"); break;
       case 0:
            //Process 2
            info(BEGIN, 2,0);
            int p2=fork();
            switch(p2)
            {
                case -1: 
                    printf("error -1"); break;
                case 0:
                    //Process 3
                    info(BEGIN, 3, 0);
                    info(END, 3, 0);
                    break;
                default:
                    //Process 2       
                                     
                    p4=fork();
                    switch(p4)
                    {
                    case -1:
                        printf("error -1"); break;
                    case 0:
                        //Process 4
                        info(BEGIN, 4, 0);
                        int p8=fork();
                        switch(p8)
                        {
                        case -1:
                            printf("error -1"); break;
                        case 0:
                            //Process 8
                            info(BEGIN, 8, 0);
            
                            pthread_t thP8[35];
                            th_str th8[35];
                            for(int i=0; i<35; i++)
                            {
                                th8[i].nrP=8;
                                th8[i].nrT=i+1;
                            }
                            for(int i=0; i<35; i++)
                            {
                                pthread_create(&thP8[i], NULL, thread_function8, &th8[i]);
                            }
                            for (int i=0; i<35; i++)
                            {
                                pthread_join(thP8[i], NULL);
                            }
                            
                            info(END, 8, 0);
                            break;
                        default:
                            //Process 4
                            waitpid(p8, 0, 0);
                            info(END, 4, 0);
                            break;
                        }
                        break;
                    default:
                        //Process 2
                        
                        p6=fork();
                        switch(p6)
                        {
                            case -1:
                                printf("error -1"); break;
                            case 0:
                                //Process 6
                                info(BEGIN, 6, 0);
                                info(END, 6, 0);
                                break;
                            default:
                                //Process 2 
                                
                                waitpid(p6, 0, 0);
                                waitpid(p4, 0, 0);
                                waitpid(p2, 0, 0);
                                info(END, 2, 0);
                                break;                           
                        }
                        break;
                    }
                    break;
            }
            
            break;
       default:
            //Process 1
            
            p5=fork();
            switch(p5)
            {
                case -1:
                    printf("error -1"); break;
                case 0:
                    //Process 5
                    info(BEGIN, 5, 0);

                    pthread_t thP5[5];
                            th_str th5[5];
                            for(int i=0; i<5; i++)
                            {
                                th5[i].nrP=5;
                                th5[i].nrT=i+1;
                            }
                            for(int i=0; i<5; i++)
                            {
                                pthread_create(&thP5[i], NULL, thread_function5, &th5[i]);
                            }
                            for (int i=0; i<5; i++)
                            {
                                pthread_join(thP5[i], NULL);
                            }

                    info(END, 5, 0);
                    break;
                default:
                    //Process 1
                    
                    p7=fork();
                    switch(p7)
                    {
                        case -1:
                            printf("error -1"); break;
                        case 0:
                            //Process 7
                            info(BEGIN, 7, 0);
                            //create threads
                            pthread_t thP7[5];
                            th_str th7[5];
                            for(int i=0; i<5; i++)
                            {
                                th7[i].nrP=7;
                                th7[i].nrT=i+1;
                            }
                            for(int i=0; i<5; i++)
                            {
                                pthread_create(&thP7[i], NULL, thread_function7, &th7[i]);
                            }
                            for (int i=0; i<5; i++)
                            {
                                pthread_join(thP7[i], NULL);
                            }

                            info(END, 7, 0);
                            break;
                        default:
                            //Process 1
                            waitpid(p7, 0, 0);
                            waitpid(p5, 0, 0);
                            waitpid(p1, 0, 0);
                            info(END, 1, 0);
                            break;
                    }
                    break;
            }
        break;       
    }
    /*
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    */
    return 0;
}
