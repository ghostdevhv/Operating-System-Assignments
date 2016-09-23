#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>


#define MAX 10480
#define TYPE1 1
#define TYPE2 2
#define TYPE3 3
#define TYPE4 4

time_t rawtime;
struct tm * timeinfo;

int no_atm;
typedef struct my_msgbuf	// ATM k saath MSG-Queue
{
	long type;
	int atm_id;
	int msgQ_id;
	int sem_id;
	int mem_seg;
	char msg[100];
	int data;

}my_msgbuf;

struct global
{
	int client_id[50];
	char timestamp[50][40];
	int balance[50];
};

struct atmlocator
{
	int atm_id[50];
	int msgQ_id[50];
	int sem_id[50];
	int mem_seg[50];
};

struct client_local	// Local Memory Structure
{
	int cid_amount[150][2];
	char type[150][20];
	char time1[150][40];

	int final_amnt[50];
	char timestamp[50][40];
	int globcid[50];
};

struct my_msgbuf atm_interact;
struct atmlocator* atm_loc;
struct global* glob_mem;
struct client_local* ptr;

int shmid_atmloc, msqid_atm, shmid_globmem;
key_t key_atm,key_atmloc,key_glob;

int checker = 1;

int glob_cons(int atm_pid, int client_pid)
{
	char timer[40];
	strcpy(timer,"0000");
	int Amunt = 0;

	for(int i=1;i<checker;i++)
	{
		if(glob_mem->client_id[i]==client_pid)
		{
			if(strcmp(timer,glob_mem->timestamp[i])<0)
			{
				strcpy(timer,glob_mem->timestamp[i]);
				Amunt = glob_mem->balance[i];
			}
		}
		else
		{

		}
	}

	for(int i=1;i<=no_atm;i++)
	{
		ptr = (struct client_local *)shmat(atm_loc->mem_seg[i],NULL,0);
		
		for(int j=1;j<150;j++)
		{
			if(ptr->cid_amount[j][0]!=0 && ptr->cid_amount[j][0] == client_pid && strcmp(ptr->time1[j],timer)>0)
			{
				if(strcmp(ptr->type[j],"DEPOSIT")==0)
				{
					Amunt+=ptr->cid_amount[j][1];
				}	
				else
				{
					Amunt-=ptr->cid_amount[j][1];
				}
			}
			
			else if(ptr->cid_amount[j][0]!=0)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		int last = shmdt((struct client_local *)ptr);
	}


	for(int i=1;i<checker;i++)
	{
		if(glob_mem->client_id[i]==client_pid)
		{
			time (&rawtime);
			timeinfo = localtime (&rawtime);
	    	strftime (glob_mem->timestamp[i],40,"%G%m%e%H%M%S",timeinfo);
	    	glob_mem->balance[i] = Amunt;
		}
		else
		{

		}
	}

	printf("Master : Done\n");
	atm_interact.type = atm_pid;
	strcpy(atm_interact.msg,"DONE");
	atm_interact.data = client_pid;
	if(msgsnd(msqid_atm,&atm_interact,sizeof(my_msgbuf),0)== -1)
    {
        perror("Send Error 134: ");
    }

		
}

int main()
{
	int b;
	key_atm = 295;
	key_atmloc = 300;
	key_glob = 400;

	printf("\nEnter the number of ATMs you want to create\n");
	scanf("%d",&no_atm);

	if((shmid_atmloc = shmget(key_atmloc, MAX, IPC_CREAT|0666)) == -1) // ATM Locator Shared File
	{
		perror("shmget ---> ATM_Locator");
		exit(1);
	}

	if((msqid_atm = msgget(key_atm,0666 | IPC_CREAT)) == -1)	// Queue for ATM Interaction
	{
		perror("msgget ---> ATM_Interact");
        exit(1);
	}

	if((shmid_globmem = shmget(key_glob, MAX, IPC_CREAT|0666)) == -1) // GLOBAL Shared Memory
	{
		perror("shmget ---> Global");
		exit(1);
	}
	
	atm_loc = (struct atmlocator *)shmat(shmid_atmloc,NULL,0); // Now master can access ATM Locator shared memory space
	atm_loc->atm_id[0] = no_atm;

	glob_mem = (struct global *)shmat(shmid_globmem,NULL,0); // Now master access global memory space

	char temp[50];
	char temp1[50];
	memset(temp,'\0',sizeof(temp));
	memset(temp1,'\0',sizeof(temp1));
	strcpy(temp,"./atm ");
	sprintf(temp1,"%d ",no_atm);
	strcat(temp,temp1);
	sprintf(temp1,"%d",shmid_globmem);
	strcat(temp,temp1);

	for(int i=1;i<=no_atm;i++)
	{
		if((b==fork())==0)
		{
			execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c",temp, (void*)NULL);
		}
		else
		{
            
		}
	}

	int count_atm = 0;
	while(1)
	{
		if(count_atm == no_atm)
		{
			break;
		}

		else if (msgrcv(msqid_atm, &atm_interact, sizeof(my_msgbuf), TYPE1, 0) >0)
        {
           	count_atm++;
           	
	        atm_loc->atm_id[count_atm] = atm_interact.atm_id;
	        atm_loc->msgQ_id[count_atm] = atm_interact.msgQ_id;
	        atm_loc->sem_id[count_atm] = atm_interact.sem_id;
	        atm_loc->mem_seg[count_atm] = atm_interact.mem_seg; 
        }

     }

	for(int i=1;i<50;i++)
	{
		if(atm_loc->atm_id[i]!=0)
		{
			printf("%d | %d | %d | %d\n",atm_loc->atm_id[i],atm_loc->msgQ_id[i], atm_loc->sem_id[i], atm_loc->mem_seg[i]);
		}
		else 
			break;
	}

	int flag = 0,atmid;
	printf("\n");
	
	while(1)
	{
		if (msgrcv(msqid_atm, &atm_interact, sizeof(my_msgbuf), TYPE2, IPC_NOWAIT) > 0)
        {
           	
           	for(int i=1;i<=no_atm;i++)
           	{
           		if(atm_loc->atm_id[i] == atm_interact.atm_id)
           		{
           			atmid = atm_interact.atm_id;
           			printf("ATM%d : Client %d Entered\n",i,atm_interact.data);
           			break;
           		}
           	}
           
           	flag = 0;
          	for(int i=1;i<checker;i++)
           	{
           		if(glob_mem->client_id[i] == atm_interact.data)
           		{
           			flag = 1;
           			break;
           		}
           	}

           	if(flag == 1)
           	{
           		atm_interact.type =  atmid;
       			strcpy(atm_interact.msg,"OLD");
   				if(msgsnd(msqid_atm,&atm_interact,sizeof(my_msgbuf),0)== -1)
			    {
			        perror("Send Error 134: ");
			    }
           	}
           	else
           	{
           		glob_mem->client_id[checker] = atm_interact.data;
           		glob_mem->balance[checker] = 0;

           		// printf("Jai Mata Di\n");
           		time (&rawtime);
				timeinfo = localtime (&rawtime);
		    	strftime (glob_mem->timestamp[checker],40,"%G%m%e%H%M%S",timeinfo);
           		checker++;
           		
           		// printf("Jai Mata Di\n");
           		atm_interact.type =  atmid;
           		// printf("%ld\n", atm_interact.type);
       			strcpy(atm_interact.msg,"NEW");
   				if(msgsnd(msqid_atm,&atm_interact,sizeof(my_msgbuf),0)== -1)
			    {
			        perror("Send Error 134: ");
			    }


           	}
        }

        if (msgrcv(msqid_atm, &atm_interact, sizeof(my_msgbuf), TYPE3, IPC_NOWAIT) > 0)
        {
        	for(int i=1;i<=no_atm;i++)
           	{
           		if(atm_loc->atm_id[i] == atm_interact.atm_id)
           		{
           			printf("ATM%d : Running a Consistency check on a/c %d\n",i,atm_interact.data);
           			break;
           		}
           	}
        	int paisa = glob_cons(atm_interact.atm_id, atm_interact.data);
        }


        if (msgrcv(msqid_atm, &atm_interact, sizeof(my_msgbuf), TYPE4, IPC_NOWAIT) > 0)
        {
        	for(int i=1;i<=no_atm;i++)
           	{
           		if(atm_loc->atm_id[i] == atm_interact.atm_id)
           		{
           			printf("ATM%d : Client %d Left\n",i,atm_interact.data);
           			break;
           		}
           	}
        }


	}
	
}