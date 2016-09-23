#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

typedef struct mmu_scheduler
{
	long mtype;
	int error;
}mmu_scheduler;

typedef struct msg_buff
{
	long mtype;
	int pid;
}msg_buff;

void msg_buff_constructor(struct msg_buff* A, long mtype, int pid)
{
	A->mtype=mtype;
	A->pid=pid;
}

void mmu_scheduler_constructor(struct mmu_scheduler* A, long mtype, int err)
{
	A->mtype=mtype;
	A->error=err;
}

int master_ki_PID,process_count,MQ1,MQ2,flag=0,num;
msg_buff receive,send;
mmu_scheduler receive1;

void loop()
{
	int abc=1;
	do
	{
		num=msgrcv(MQ1,&receive,sizeof(receive),1,0);
		usleep(50000);
		if(!(num>=0))
		{
			perror("Error in ready queue1: ");
			exit(0);
		}
		int sch_pid=receive.pid;
		printf("Scheduling process:  %d\n",receive.pid);
		kill(receive.pid,SIGUSR1);
		num=msgrcv(MQ2,&receive1,sizeof(receive1),0,0);
		if(!(num>=0))
		{
			perror("Error in ready queue2: ");
			exit(0);
		}
		int err=receive1.error;
		printf("Scheduling error:  %d\n",err);
		if(err==1)
		{
			printf("Page fault handled by %d process\n",sch_pid);
			msg_buff_constructor(&send,1,sch_pid);
			// send.mtype=1;
			// send.pid=sch_pid;
			num=msgsnd(MQ1,&send,sizeof(send),0);
			if(num<0)
			{
				perror("Error in insertion in ready queue: ");
				exit(0);
			}
		}
		else if(err==2)
		{
			printf("Process %d terminated\n",sch_pid);
			flag+=5-4;
			printf("flag = %d and num = %d\n",flag,process_count);
			if(flag==process_count)
			{
				printf("kill\n");
				kill(master_ki_PID,SIGUSR1);
			}
		}
	}while(abc<10);
}

int main(int argc,char *argv[])
{
	char a[100],b[100],c[100],d[100];
	strcpy(a, argv[1]);
	strcpy(b, argv[2]);
	strcpy(c, argv[3]);
	strcpy(d, argv[4]);

	master_ki_PID=atoi(c);
	process_count=atoi(d);
	MQ1=atoi(a);
	MQ2=atoi(b);


	printf("Scheduled memory queues are %d and %d\n",MQ1,MQ2);

	loop();
	return 0;

}
