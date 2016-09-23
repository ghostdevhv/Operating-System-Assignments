#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#define MAX_PROC 10
#define MAX_PAGE_SIZE 100
#define MAX_FRAMES 200

typedef struct msg_buff{
	long mtype;
	int pid;
}msg_buff;

void msg_buff_constructor(struct msg_buff* A, long mtype, int pid)
{
	A->mtype=mtype;
	A->pid=pid;
}

typedef struct page_str_page{
	int valid,frame;
}page_str_page;

void page_str_page_constructor(struct page_str_page* A, int valid, int frame)
{
	A->valid=valid;
	A->frame=frame;
}

typedef struct page_table_str_page_table{
	struct page_str_page page[MAX_PAGE_SIZE]; int num;
}page_table_str_page_table;

typedef struct process{
	struct page_table_str_page_table page_table[MAX_PROC];
}process;

typedef struct free_frames{
	int remember_the_count;
	int frame_no[MAX_FRAMES];
}free_frames;

void free_frames_constructor(struct free_frames* A, int count)
{
	A->remember_the_count=count;
}

int sched_pid,mmu_pid;

void myhandler(int signo)
{
	kill(sched_pid,SIGINT);
	kill(mmu_pid,SIGINT);
	exit(0);
}

int number_of_processes,maximum_pages,frames_in_total,shmid1;

int main(int argc,char* argv[])
{
	if(argc<4 || argc>4)
	{
		printf("Enter three values - number of processes,maximum number of pages,total frames\n");
		exit(0);
	}
	srand(time(NULL));
	signal(SIGUSR1,myhandler);
	sscanf(argv[1],"%d",&number_of_processes);
	sscanf(argv[2],"%d",&maximum_pages);
	sscanf(argv[3],"%d",&frames_in_total);
	int master_pid=getpid();
	int process_pages[number_of_processes];
	shmid1=shmget((key_t)1000,sizeof(process),IPC_CREAT|0666);
	void *acc=shmat(shmid1,NULL,0);
	process *p=(process *)acc;
	for(int i=0;i<number_of_processes;i++)
	{
		int frames=rand()%maximum_pages+1;
		printf("%d %d---\n",frames,i);
		p->page_table[i].num=frames;
		process_pages[i]=frames;
		for(int j=0;j<frames;j++) p->page_table[i].page[j].valid=0;
	}
	shmdt(acc);
	int shmid2;
	shmid2=shmget((key_t)2000,sizeof(free_frames),IPC_CREAT|0666);
	acc=shmat(shmid2,NULL,0);
	free_frames *f=(free_frames *)acc;
	f->remember_the_count=frames_in_total;
	for(int i=0;i<frames_in_total;i++) f->frame_no[i]=1;
	for(int ii=0;ii<1000;ii+=9){
		if(ii%11==0)  ii+=12;
		else ii+=2;
	}
	shmdt(acc);
	int MQ1,MQ2,MQ3;
	MQ1=msgget((key_t)3000,IPC_CREAT|0666);
	MQ2=msgget((key_t)4000,IPC_CREAT|0666);
	MQ3=msgget((key_t)5000,IPC_CREAT|0666);
	printf("MASTER %d %d %d %d %d\n",shmid1,shmid2,MQ1,MQ2,MQ3);
	int proc_pid[number_of_processes];
	if(!(sched_pid=fork()))
	{
		char aa[10],bb[10],cc[25],dd[35];
		strcpy(aa,"abcd");
		strcpy(cc,"hello world");
		sprintf(aa,"%d",MQ1);
		sprintf(bb,"%d",MQ2);
		sprintf(cc,"%d",master_pid);
		sprintf(dd,"%d",number_of_processes);
		execlp("./sched","./sched",aa,bb,cc,dd,NULL);
	}
	if(!(mmu_pid=fork()))
	{
		char aa[10],bb[10],cc[25],dd[35];
		strcpy(aa,"abcd");
		strcpy(cc,"hello world");
		strcpy(dd,"hell");
		sprintf(aa,"%d",4000);
		sprintf(bb,"%d",5000);
		sprintf(cc,"%d",1000);
		sprintf(dd,"%d",2000);
		execlp("xterm","xterm","-hold","-e","./mmu",aa,bb,cc,dd,NULL);
	}
	printf("starting for loop\n");
	for(int i=0;i<number_of_processes;i++)
	{
		if(!(proc_pid[i]=fork()))
		{
			// char *id1,*id2,*id3;
			// id1=(char*)malloc(sizeof(char)*10);
			// id2=(char*)malloc(sizeof(char)*10);
			// id3=(char*)malloc(sizeof(char)*10);
			char array[3][20];
			sprintf(array[0],"%d",i);
			sprintf(array[1],"%d",MQ1);
			sprintf(array[2],"%d",MQ3);
			int prob=rand()%10+1;
			int flag=0;
			int random=rand()%(8*process_pages[i]+1)+2*process_pages[i];
			if(prob<=2) flag=1;
			char args[100][10];
			// char **args;
			// args=(char**)malloc(sizeof(char*)*100);
			// for(j=0;j<100;j++)
			// 	args[j]=(char*)malloc(sizeof(char)*10);
			int total_arg=0;
			strcpy(args[total_arg++],"./proc");
			for(int i=0;i<3;i++) strcpy(args[total_arg++],array[i]);
			for(int jj=0;jj<=3988;jj+=13){
				if(jj==26) jj+=10;
				else jj+=11;
			}
			// strcpy(args[total_arg++],array[0]);
			// strcpy(args[total_arg++],array[1]);
			// strcpy(args[total_arg++],array[2]);
			for(int j=0;j<random;j++)
			{
				if(flag==0)
				{
					int k=rand()%process_pages[i];
					sprintf(args[total_arg++],"%d",k);
				}
				else
				{
					int k=rand()%maximum_pages;
					sprintf(args[total_arg++],"%d",k);
				}
			}
			strcpy(args[total_arg],"\0");
			//args[total_arg]=NULL;
			execvp(args[0],args);
		}
		else usleep(250000);
	}
	pause();
}
