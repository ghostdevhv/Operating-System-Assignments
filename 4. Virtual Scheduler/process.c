#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define TYPE2 2
#define TYPE 1
key_t key = 270;

typedef struct my_msgbuf
{
    long type;
    int priority_info;
   	pid_t pid;
    int process_num;
    char text[100];
}my_msgbuf;

int flag = 0;

static void handler()		// Pause
{
	printf("********** Got Notify Signal  **********\n");
}

static void handler2()	// Time quant expires
{
	printf("Time Quant Expired (Signal Received) for Process having PID : %d \n",(int)getpid());
	flag = 1;
}

int main(int argc , char *argv[])
{
	signal(SIGUSR1,handler); // Pause hatane k liye
	signal(SIGUSR2,handler2); // Time Quanta expire huya ye btane k liye
	srand(time(NULL));
	if(argc!=6)
	{
		perror("Error: ");
	}

	/* For printing into file*/
	// char input[200];
	 //    sprintf(input,"ab%d.txt",getpid());
	 //    FILE *ptr=fopen(input,"w");
	 //    int fd=fileno(ptr);
	 //    dup2(fd,1);

	struct my_msgbuf proc;
	int msqid;
	int i=0,j;
	int sched_pid;

	int priority = atoi(argv[1]);
	int iterations = atoi(argv[2]); 
	int sleep_time = atoi(argv[3]);
	double sleep_prob = atof(argv[4]);
	int process_num = atoi(argv[5]);

	if((msqid = msgget(key,IPC_CREAT|0666))== -1)
	{
		perror("msgget");
		exit(1);
	}

	proc.pid=getpid();
	proc.priority_info = priority;
	proc.type = TYPE2;
	proc.process_num =  process_num;

	if(msgsnd(msqid,&proc,sizeof(my_msgbuf),0)==-1)
	{
		perror("Send Error 69 : ");
	}

	printf("%d | %d | %ld | %d\n",proc.pid,proc.priority_info,proc.type,proc.process_num);
	
	
	if(msgrcv(msqid, &proc, sizeof(my_msgbuf),getpid(), 0) == -1)
	{
		perror("Recieve Error 78 : ");
	}

	sched_pid = proc.pid;
    
	labelA: while(1)
	{
		pause();

		while(1)
		{
			printf("\n\n");
			if(flag==1)
			{
				flag = 0;
				goto labelA;
			}	
			if(i>=iterations)
			{
				kill(sched_pid,SIGUSR2);
				printf("Iterations Complete | Process Terminated\n");
				pause();
			}
			printf("Iteration Number  ------ %d\n",i+1);
			double prob = ((double) rand() / (RAND_MAX));
			printf("%lf\n",prob);
			if(sleep_prob>=prob)
			{
				printf("I/O Waiting | Going for Sleep --> %d\n",process_num);
				kill(sched_pid,SIGUSR1);
				sleep(sleep_time);

				proc.pid=getpid();
				proc.priority_info = priority;
				proc.type = TYPE;
				proc.process_num = process_num;
				strcpy(proc.text,"I/O Complete");
				if(msgsnd(msqid,&proc,sizeof(my_msgbuf),0)==-1)
				{
					perror("Send Error 116: ");
				}
				printf("I/O Completed for Process Number %d --> PID %d\n",process_num,(int)getpid());
				i++;
				break;
			}
			i++;
		}
	}
	getchar();

}