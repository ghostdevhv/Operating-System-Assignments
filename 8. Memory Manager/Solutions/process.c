#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
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

void handler(int signo)
{
	printf("Process %d is terminaed and is starting again\n",getpid());
	return ;
}

typedef struct message_buff
{
	long mtype;
	int page_number;
	int myindex;
}message_buff;

typedef struct mmu_p
{
	long mtype;
	int err;
}mmu_p;

typedef struct msg_buff{
	long mtype;
	int pid;
}msg_buff;

void msg_buff_constructor(struct msg_buff* A, long mtype, int pid)
{
	A->mtype=mtype;
	A->pid=pid;
}

void message_buff_constructor(struct message_buff* A, long mtype, int page_number, int myindex)
{
	A->mtype=mtype;
	A->page_number=page_number;
	A->myindex=myindex;
}
void mmu_p_constructor(struct mmu_p* A, long mtype, int err)
{
	A->mtype=mtype;
	A->err=err;
}

int refer[100],myindex,number1,MQ1,MQ3,flag=0,length=0;
msg_buff send1;
message_buff send;
mmu_p recv;

void loop()
{
	while(length<flag)
	{
		printf("Page %d requested by process %d\n",refer[length],getpid() );
		message_buff_constructor(&send,1,refer[length],myindex);
		// send.mtype=1,send.myindex=myindex;
		// send.page_number=refer[length];
		number1=msgsnd(MQ3,&send,sizeof(send),0);
		if(number1<0)
		{
			perror("Sending error :");
			exit(0);
		}
		printf("Message sent  successfully\n");
		number1=msgrcv(MQ3,&recv,100000,2,0);
		if(number1<0)
		{
			perror("Error in recv message: ");
			exit(0);
		}
		int num=recv.err;
		printf("number recvieved %d\n",num);
		if(num==-1){
			printf("Page fault occured for process = %d and page no = %d\n",getpid(),refer[length]);
			pause();
		}
		else{
			if(num==-2){
				printf("Invalid frame requested\n"); exit(0);
			}
			else ++length;
		}
	}
	return;
}

int main(int argc,char *argv[])
{
	signal(SIGUSR1,handler);
	MQ3=atoi(argv[3]);
	myindex=atoi(argv[1]);
	MQ1=atoi(argv[2]);
	printf("myindex = %d MQ1 = %d MQ3 = %d\n reference string\n",myindex,MQ1,MQ3);
	int total_args=argc;
	for(int i=4;i<total_args;i++){
		refer[flag++]=atoi(argv[i]);
	}
	for(int i=4;i<flag;i++)
		printf("%d ",refer[i]);
	printf("\n");
	msg_buff_constructor(&send1,1,getpid());
	// send1.pid=getpid();
	// send1.mtype=1;
	number1=msgsnd(MQ1,&send1,sizeof(send1),0);
	if(number1<0)
	{
		perror("Error in sending msg: ");
	}
	pause();
	loop();
	message_buff_constructor(&send,1,-9,myindex);
	// send.mtype=1;
	// send.page_number=-9;
	// send.myindex=myindex;
	number1=msgsnd(MQ3,&send,sizeof(send),0);
	return 0;
}
