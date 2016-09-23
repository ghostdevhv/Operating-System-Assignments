#include <bits/stdc++.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#define token_delimiters " "
typedef struct my_msgbuf
{
    long type;
    pid_t pid;
    int text;
}my_msgbuf;
void updatematrix(int,int,int);
using namespace std;
struct msqid_ds qstat1,qstat2;
struct sembuf sop;
struct my_msgbuf q1,q2;
int semid;

int main(int argc, char* argv[])
{
    srand(time(NULL));
    int process_num,msqid1,msqid2;
    key_t key1=atoi(argv[1]);
    key_t key2=atoi(argv[2]);
    key_t key_sem=atoi(argv[3]);
    process_num=atoi(argv[4]);
    printf("consumer=%d\n",process_num);
    double p=0.7;

    if ((msqid1 = msgget(key1, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    if ((msqid2 = msgget(key2, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    if((semid = semget(key_sem,9,0666|IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    while(1){
        double t=1.0*rand()/INT_MAX;
        int x;
        if(t<=p){  // consume from 1 queue
            printf("\nOkay consuming from 1 queue\n");
            if(rand()%2==0) x=1;  // queue1
            else x=2;  // queue2
            printf("Trying to consume from queue %d\n",x);
            sop.sem_num=2;
            sop.sem_op=-1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // get lock on file matrix.txt
            printf("got lock on file 65\n");
            updatematrix(process_num,x,1);  // process i has requested for queueX
            printf("matrix updated 67\n");
            sop.sem_num=2;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // release lock on file matrix.txt
            printf("release lock on file 72\n");
            sop.sem_num=(x==1)?4:6;
            sop.sem_op=-1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // check if Q is empty
            printf("queue %d is not empty\n",x);
            sop.sem_num=(x==1)?0:1;
            sop.sem_op=-1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // acquire lock on Q1
            printf("got lock on queue %d 77\n",x);
            sop.sem_num=2;
            sop.sem_op=-1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // get lock on file matrix.txt
            printf("got lock on file 82\n");
            updatematrix(process_num,x,2);  // process i acquired lock of queueX
            printf("matrix updated 84\n");
            sop.sem_num=2;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // release lock on file matrix.txt
            printf("release lock on file 89\n");
            if(x==1){
                if(msgrcv(msqid1,&q1,sizeof(my_msgbuf),1,0)==-1){
                    perror("Error 59: ");
                }
                printf("Process %d consumes %d from Queue1\n",process_num,q1.text);
            }
            else{
                if(msgrcv(msqid2,&q2,sizeof(my_msgbuf),1,0)==-1){
                    perror("Error 59: ");
                }
                printf("Process %d consumes %d from Queue2\n",process_num,q2.text);
            }
            sop.sem_num=(x==1)?0:1;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // release lock on Q1
            printf("released lock from queue %d 106\n",x);
            sop.sem_num=2;
            sop.sem_op=-1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // get lock on file matrix.txt
            printf("got lock on file 111\n");
            updatematrix(process_num,x,0);  // process i released lock of queueX
            sop.sem_num=2;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // release lock on file matrix.txt
            printf("release lock on file 117\n");
            sop.sem_num=(x==1)?3:5;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // update queue1 full
            sop.sem_num=8;
            sop.sem_op=1;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // update deletion
            printf("consumption completed from queue %d\n\n",x);
        }
        else{   // consume from 2 queues
            printf("\nOkay consuming from 2 queues\n");
            int temp[2]={1,2};
            int x;
            //if(rand()%2==1) swap(temp[0],temp[1]);
            for(int i=0;i<=1;i++){
                x=temp[i];
                printf("Trying to consume from queue %d\n",x);
                sop.sem_num=2;
                sop.sem_op=-1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // get lock on file matrix.txt
                printf("got lock on file 140\n");
                updatematrix(process_num,x,1);  // process i has requested for queueX
                sop.sem_num=2;
                sop.sem_op=1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // release lock on file matrix.txt
                printf("release lock on file 146\n");
                sop.sem_num=(x==1)?4:6;
                sop.sem_op=-1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // check if Q is empty
                printf("queue %d is not empty\n",x);

                sop.sem_num=(x==1)?0:1;
                sop.sem_op=-1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // acquire lock on Q1
                printf("got lock on queue %d 151\n",x);
                sop.sem_num=2;
                sop.sem_op=-1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // get lock on file matrix.txt
                printf("got lock on file 156\n");
                updatematrix(process_num,x,2);  // process i acquired lock of queueX
                sop.sem_num=2;
                sop.sem_op=1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // release lock on file matrix.txt
                printf("release lock on file 163\n");
                if(x==1){
                    if(msgrcv(msqid1,&q1,sizeof(my_msgbuf),1,0)==-1){
                        perror("Error 59: ");
                    }
                    printf("Process %d consumes %d from Queue1\n",process_num,q1.text);
                }
                else{
                    if(msgrcv(msqid2,&q2,sizeof(my_msgbuf),1,0)==-1){
                        perror("Error 59: ");
                    }
                    printf("Process %d consumes %d from Queue2\n",process_num,q2.text);
                }
            }
            x=3-x;  // if x==2 we need to release lock on q1 first
            for(int i=0;i<2;i++){
                sop.sem_num=(x==1)?0:1;
                sop.sem_op=1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // release lock on Q1
                printf("released lock from queue %d 184\n",x);
                sop.sem_num=2;
                sop.sem_op=-1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // get lock on file matrix.txt
                printf("got lock on file 190\n");
                updatematrix(process_num,x,0);  // process i released lock of queueX
                sop.sem_num=2;
                sop.sem_op=1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // release lock on file matrix.txt
                printf("release lock on file 197\n");
                sop.sem_num=(x==1)?3:5;
                sop.sem_op=1;
                sop.sem_flg=0;
                semop(semid,&sop,1);  // update queueX full
                x=3-x;
            }
            sop.sem_num=8;
            sop.sem_op=2;
            sop.sem_flg=0;
            semop(semid,&sop,1);  // update deletion
            printf("consumed from both queues\n");
        }

    }
    getchar();
}

void updatematrix(int col, int row, int val)
{
    FILE *ptr=fopen("matrix.txt","r");
    int mat[3][11];
    memset(mat,0,sizeof(mat));
    for(int i=1;i<=2;i++)
        for(int j=1;j<=10;j++){
            fscanf(ptr,"%d",&mat[i][j]);
            //printf("mat[%d][%d]=%d\n",i,j,mat[i][j]);
        }
    mat[row][col]=val;
    printf("updated mat[%d][%d]=%d\n",row,col,val);
    fclose(ptr);
    ptr=fopen("matrix.txt","w");
    for(int i=1;i<=2;i++){
        for(int j=1;j<=10;j++)
            fprintf(ptr,"%d ",mat[i][j]);
        fprintf(ptr, "\n");
    }
    fclose(ptr);
}
