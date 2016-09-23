#include <bits/stdc++.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>


using namespace std;

#define token_delimiters " "


typedef struct my_msgbuf
{
    long type;
    pid_t pid;
    int text;
}my_msgbuf;

struct sembuf sop;
struct my_msgbuf q1,q2;
int semid;
void updatematrix(int,int,int);

int main(int argc, char* argv[])
{
    srand(time(NULL));
	key_t key1 = atoi(argv[1]);	// Queue1
	key_t key2 = atoi(argv[2]); // Queue2
	key_t key_sem = atoi(argv[3]);
    int process_num = atoi(argv[4]);
    printf("process_num=%d\n",process_num);
    printf("process num=%d pid=%d\n",process_num,getpid());
    struct msqid_ds qstat1,qstat2;
	int msqid1,msqid2;
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
    /*
        0 -> Queue1
        1 -> Queue2
        2 -> matrix.txt
        3 -> queue1 full  init to 10
        4 -> queue1 empty  init to 0
        5 -> queue2 full  init t0 10
        6 -> queue2 empty init to 0
        7 -> insertion
        8 -> deletion
    */
    int x;
    while(1)
    {
        if(rand()%2==0) x=1;
        else x=2;
        printf("\nTrying to insert in queue %d\n",x);
        sop.sem_num=(x==1)?3:5;
        sop.sem_op=-1;
        sop.sem_flg=IPC_NOWAIT;
        int zz=semop(semid,&sop,1);  // check if Qx is full
        if(zz==-1 and errno==EAGAIN){
            printf("Queue %d is full. Continue...\n",x);
            continue;
        }
        printf("queue %d is not full\n",x);
        sop.sem_num=2;
        sop.sem_op=-1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // get lock on file matrix.txt
        printf("got lock on file\n");
        int z=semctl(semid,2,GETVAL,0);
        printf("sem2=%d\n",z);
        updatematrix(process_num,x,1);  // process i has requested for queueX
        printf("matrix updated 83\n");
        sop.sem_num=2;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // release lock on file matrix.txt
        printf("release lock on file 88\n");
        z=semctl(semid,2,GETVAL,0);
        printf("sem2=%d\n",z);
        sop.sem_num=(x==1)?0:1;
        sop.sem_op=-1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // acquire lock on Q1
        printf("got lock on queue %d 93\n",x);
        sop.sem_num=2;
        sop.sem_op=-1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // get lock on file matrix.txt
        printf("got lock on file 98\n");
        updatematrix(process_num,x,2);  // process i acquired lock of queueX
        printf("matrix updated 100\n");
        sop.sem_num=2;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // release lock on file matrix.txt

        if(x==1){
            q1.text=rand()%50+1;
            q1.type=1;
            if(msgsnd(msqid1,&q1,sizeof(my_msgbuf),0)==-1){
                perror("Error 106: ");
            }
            printf("Producer %d Successfully inserted %d in queue 1\n",process_num,q1.text);
        }
        else{
            q2.text=rand()%50+1;
            q2.type=1;
            if(msgsnd(msqid2,&q2,sizeof(my_msgbuf),0)==-1){
                perror("Error 114: ");
            }
            printf("Producer %d Successfully inserted %d in queue 2\n",process_num,q2.text);
        }
        sop.sem_num=(x==1)?0:1;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // release lock on Qx
        printf("released lock from queue %d 184\n",x);
        sop.sem_num=2;
        sop.sem_op=-1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // get lock on file matrix.txt
        printf("got lock on file 137\n");
        updatematrix(process_num,x,0);  // process i released lock of queueX
        sop.sem_num=2;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // release lock on file matrix.txt
        printf("release lock on file 143\n");
        sop.sem_num=(x==1)?4:6;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // update queueX empty
        sop.sem_num=7;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // update insertion
        int xx=(rand()%4+1);
        printf("Producer %d going to sleep for %d seconds\n",process_num,xx);
        //sleep(1);
    }
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
