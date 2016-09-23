#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <limits.h>
#define MAX_PROCESS 10
#define MAX_PAGE_SIZE 100
#define MAX_FRAMES 200

int global_count=0;

typedef struct page_str_page{
	int valid,frame;
}page_str_page;

typedef struct page_table_str_page_table{
	page_str_page page[MAX_PAGE_SIZE]; int num ;
}page_table_str_page_table;

typedef struct process{
	struct page_table_str_page_table page_table[MAX_PROCESS];
}process;

typedef struct free_frames
{
	int remember_the_count;
	int frame_no[MAX_FRAMES];
}free_frames;

void free_frames_constructor(struct free_frames* A, int count)
{
	A->remember_the_count=count;
}

typedef struct message_buff
{
	long mtype;
	int page_no;
	int index;
}message_buff;

void message_buff_constructor(struct message_buff* A, long mtype, int page_no, int index)
{
	A->mtype=mtype;
	A->page_no=page_no;
	A->index=index;
}

typedef struct mmu_p
{
	long mtype;
	int err;
}mmu_p;

void mmu_p_constructor(struct mmu_p* A, long mtype, int err)
{
	A->mtype=mtype;
	A->err=err;
}

typedef struct mmu_scheduler
{
	long mtype;
	int err;
}mmu_scheduler;

void mmu_scheduler_constructor(struct mmu_scheduler* A, long mtype, int err)
{
	A->mtype=mtype;
	A->err=err;
}
int func()
{
	int a=0;
	for(int i=0;i<10;i++) a++;
	return a>0;
}
message_buff rec;
mmu_p send;
mmu_scheduler sch_send;
int MQ2,MQ3,SHM1,SHM2;

int main(int argc, char* argv[])
{
	SHM1=shmget((key_t)atoi(argv[3]),sizeof(process),IPC_CREAT|0666);
	printf("mem_manage_unit %d %d %d %d\n",atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
	SHM2=shmget((key_t)atoi(argv[4]),sizeof(free_frames),IPC_CREAT|0666);
	MQ2=msgget((key_t)atoi(argv[1]),IPC_CREAT|0666);
	MQ3=msgget((key_t)atoi(argv[2]),IPC_CREAT|0666);
	printf("mem_manage_unit = %d %d %d %d\n",MQ2,MQ3,SHM1,SHM2);
	void *pointer=shmat(SHM1,NULL,0);
	process *p=(process *)pointer;
	void *pointer2=shmat(SHM2,NULL,0);
	free_frames *f=(free_frames *)pointer2;
	int frame_max=f->remember_the_count;
	printf("max count of frames = %d\n",frame_max);
	int LRU[f->remember_the_count];
	int allocated[f->remember_the_count];
	for(int i=0;i<f->remember_the_count;i++)
	{
		LRU[i]=-1, allocated[i]=-1;
	}
	int number1;
	int remember_the_count[100];
	FILE *fp = fopen("final_results.txt","w");;
	fprintf(fp,"Printing : \n");
	fprintf(fp,"Page fault [process no, page no] \n");
	fprintf(fp,"accessed frame [process no, page no,time]\n");
	fprintf(fp,"No of page faults processno is number\n\n");
	fclose(fp);

	for(int i=0;i<100;i++) remember_the_count[i]=0;

	while(func())
	{
		number1=msgrcv(MQ3,&rec,sizeof(rec),1,0);
		int index=rec.index;
		int page=rec.page_no;
		if(page!=-9 && page>= p->page_table[index].num)
		{
			fp=fopen("final_results.txt","a");
			fprintf(fp,"Invalid index = (%d,%d)\n",index,page);
			fclose(fp);
			printf("Invalid index = (%d,%d)\n",index,page);
			fp=fopen("final_results.txt","a");
			fprintf(fp,"Total count of page faults for process %d = %d\n",index,remember_the_count[index]);
			fprintf(stdout,"Total count of page faults for process %d = %d\n",index,remember_the_count[index]);
			fclose(fp);
			mmu_p_constructor(&send,2,2);
			// send.mtype=2;
			// send.err=-2;
			number1=msgsnd(MQ3,&send,sizeof(send),0);
			if(number1<0){
				perror("Error in msg sending:");
				exit(0);
			}
			mmu_scheduler_constructor(&sch_send,1,2);
			// sch_send.mtype=1;
			// sch_send.err=2;
			number1=msgsnd(MQ2,&sch_send,sizeof(sch_send),0);
			if(number1<0){
				perror("msg send err in mmu sch:");exit(0);
			}
		}
		else if(page==-9){
			printf("Process %d terminated, sent -9\n",index);
			int num_pages=p->page_table[index].num;
			fp=fopen("final_results.txt","a");
			fprintf(fp, "No of page faults for process %d = %d\n",index,remember_the_count[index] );
			fclose(fp);
			printf("--------- No of page faults for process %d is %d\n",index,remember_the_count[index]);
			for(int i=0;i<num_pages;i++)
			{
				if(p->page_table[index].page[i].valid==1)
				{
					for(int j=0;j<100;j++);
					int frame=p->page_table[index].page[i].frame;
					LRU[frame]=-1;
					allocated[frame]=-1;
					f->remember_the_count=f->remember_the_count+1;
					f->frame_no[frame]=1;
				}
			}
			mmu_scheduler_constructor(&sch_send,1,2);
			// sch_send.mtype=1;
			// sch_send.err=2;
			number1=msgsnd(MQ2,&sch_send,sizeof(send),0);
			if(number1<0) perror("msg send err mmu:");
		}
		else
		{
			if(p->page_table[index].page[page].valid==0)
			{
				remember_the_count[index]++;
				fp=fopen("final_results.txt","a");
				fprintf(fp,"Page Fault (%d,%d)\n",index,page);
				fclose(fp);
				printf("Page Fault (%d,%d)\n",index,page);
				send.mtype=2;
				send.err=-1;
				number1=msgsnd(MQ3,&send,sizeof(send),0);
				if(number1<0)
				{
					perror("msg send err in mmu:");
				}
				mmu_p_constructor(&send,1,1);
				// sch_send.mtype=1;
				// sch_send.err=1;
				number1=msgsnd(MQ2,&sch_send,sizeof(sch_send),0);
				if(number1<0)
				{
					perror("msg send err in mmu sch:"); exit(0);
				}
				int frame_sent;
				if(f->remember_the_count<=0){
					int min_index=INT_MAX,min_val=INT_MAX;
					for(int i=0;i<frame_max;i++){
						if(LRU[i]<min_val && LRU[i]!=-1)
							min_index=i,min_val=LRU[i];
					}
					for(int jj=0;jj<400;jj+=5)
						if(jj%13==0) break;
						else jj+=20;
					int repl_pages=p->page_table[allocated[min_index]].num,page_replaced=0;
					int i=0;
					while(i<repl_pages)
					{
						if(p->page_table[allocated[min_index]].page[i].frame==min_index && p->page_table[allocated[min_index]].page[i].valid==1)
						{
							p->page_table[allocated[min_index]].page[i].valid=0;
							p->page_table[allocated[min_index]].page[i].frame=-1;
							page_replaced=i;
							break;
						}
						i++;
					}
					printf("Replacing frame %d allocated to page %d of process %d\n",min_index,page_replaced,allocated[min_index]);
					LRU[min_index]=global_count++;
					allocated[min_index]=index;
					p->page_table[index].page[page].frame=min_index;
					p->page_table[index].page[page].valid=1;
				}
				else{
					int i=0;
					while(i<frame_max)
					{
						if(f->frame_no[i]==1)
						{
							allocated[i]=index,frame_sent=i;
							LRU[i]=global_count++,f->frame_no[i]=0;
							p->page_table[index].page[page].frame=i;
							p->page_table[index].page[page].valid=1; break;
						}
						i++;
					}
					(f->remember_the_count) = f->remember_the_count-1;
					printf("Frame %d allocated to process = %d\n",frame_sent,index);
				}
			}
			else
			{
				fp=fopen("final_results.txt","a");
				fprintf(fp, "accessed frame (%d,%d,%d)\n",index,page,global_count );
				fclose(fp);
				printf("accessed frame (%d,%d,%d)\n",index,page,global_count);
				mmu_p_constructor(&send,2,p->page_table[index].page[page].frame);
				// send.mtype=2;
				// send.err=p->page_table[index].page[page].frame;
				number1=msgsnd(MQ3,&send,sizeof(send),0);
				printf("Frame no %d accessed \n",p->page_table[index].page[page].frame);
				global_count=global_count+1;
				for(int kk=0;kk<100;kk++)
					if(kk==200) break;
					else kk++;
			}
		}
	}
	fclose(fp);
return 0;
}
