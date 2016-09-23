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

struct databuf	// Local Memory Structure
{
	int cid_amount[150][2];
	char type[150][20];
	char time1[150][40];

	int final_amnt[50];
	char timestamp[50][40];
	int globcid[50];
};

struct atmlocator
{
	int atm_id[50];
	int msgQ_id[50];
	int sem_id[50];
	int mem_seg[50];
};

typedef struct master_msgbuf	// Master k saath msg queue
{
	long type;
	int atm_id;
	int msgQ_id;
	int sem_id;
	int mem_seg;
	char msg[100];
	int data;

}master_msgbuf;

typedef struct client_msgbuf	// Client k saath Msq Queue
{
	long type;
	int pid;
	char msg[100];
	int data;

}client_msgbuf;

struct global_master
{
	int client_id[50];
	char timestamp[50][40];
	int balance[50];
};

key_t key_master = 295;
key_t key_client;
key_t key_sem = 240;
key_t key_atmloc= 300;
key_t key_local;

int msqid_master, semid_lock, shmid_atmloc, msqid_client, shmid_local;
int nop;

int temp_cid_amount[150][2];
char temp_type[150][20];
char temp_time1[150][40];

struct master_msgbuf master_interact;
struct client_msgbuf client_interact;
struct sembuf sop;
struct databuf* local_mem;
struct databuf* ptr;
struct atmlocator* atm_loc;
struct global_master* master;

int localconsis(int money, int pidoo)
{
	char timer[40];
	strcpy(timer,"0000");
	int Amunt = 0;
	
	/* In 2nd table*/
	for(int i=1;i<=nop;i++)
	{
		if(atm_loc->atm_id[i]!=(int)getpid())
		{
			ptr = (struct databuf *)shmat(atm_loc->mem_seg[i],NULL,0);
			
			for(int j=1;j<50;j++)
			{
				if(ptr->globcid[i]!=0 && ptr->globcid[i] == pidoo)
				{
					if(strcmp(ptr->timestamp[i],timer)>0)
					{
						strcpy(timer,ptr->timestamp[i]);
						Amunt = ptr->final_amnt[i];
					}
				}
				
				else
				{
					break;
				}
			}
			int last = shmdt((struct databuf *)ptr);

		}

		else
		{
			for(int j=1;j<50;j++)
			{
				if(local_mem->globcid[i]!=0 && local_mem->globcid[i] == pidoo )
				{
					if(strcmp(local_mem->timestamp[i],timer)>0)
					{
						strcpy(timer,local_mem->timestamp[i]);
						Amunt = local_mem->final_amnt[i];
					}
				}
				
				else
				{
					break;
				}
			}
		}
	}

	/* 2nd table me checking khatm ho gyi :P */

	/* Ab first table me krna hai :/*/
	for(int i=1;i<=nop;i++)
	{
		if(atm_loc->atm_id[i]!=(int)getpid())
		{
			ptr = (struct databuf *)shmat(atm_loc->mem_seg[i],NULL,0);
			
			for(int j=1;j<150;j++)
			{
				if(ptr->cid_amount[j][0]!=0)
				{
					if(ptr->cid_amount[j][0] == pidoo && strcmp(ptr->time1[j],timer)>0)
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
				}
				else
				{
					break;
				}
			}

			int last = shmdt((struct databuf *)ptr);

		}

		else
		{
			for(int j=1;j<150;j++)
			{
				if(local_mem->cid_amount[j][0]!=0)
				{
					if(local_mem->cid_amount[j][0] == pidoo && strcmp(local_mem->time1[j],timer)>0)
					{
						if(strcmp(local_mem->type[j],"DEPOSIT")==0)
						{
							Amunt+=local_mem->cid_amount[j][1];
						}
						else
						{
							Amunt-=local_mem->cid_amount[j][1];
						}
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return Amunt;

}

int main(int argc, char* argv[])
{
	nop = atoi(argv[1]);

	key_client = (int)getpid() + 100;
	key_local = (int)getpid() + 200;

	
	if((msqid_master = msgget(key_master,0666 | IPC_CREAT)) == -1)	// Queue for Master Interaction
	{
		perror("msgget ---> Master_Interact");
    	exit(1);
	}
	
	if((msqid_client = msgget(key_client,0666 | IPC_CREAT)) == -1)	// Queue for Client Interaction
	{
		perror("msgget ---> Client_Interact");
        exit(1);
	}

	if((semid_lock = semget(key_sem, nop, 0666|IPC_CREAT)) == -1) // Semaphore for atm lock
    {
        perror("semget --> Atm_Lock"); 
        exit(1);
    }

    if((shmid_local = shmget(key_local, MAX, IPC_CREAT|0666)) == -1) // Local Memory Space
	{
		perror("shmget ---> Local Memory");
		exit(1);
	}

	local_mem = (struct databuf *)shmat(shmid_local,NULL,0); // Now ATM access local memory space

	if((shmid_atmloc = shmget(key_atmloc, MAX, IPC_CREAT|0666)) == -1) // ATM Locator Shared File
	{
		perror("shmget ---> ATM_Locator");
		exit(1);
	}

	atm_loc = (struct atmlocator *)shmat(shmid_atmloc,NULL,0); // Now ATM can access ATM Locator shared memory space


	for(int i=0;i<nop;i++)
	{
		semctl(semid_lock,i,SETVAL,1);
	}

	master_interact.type = TYPE1;
	master_interact.atm_id = (int)getpid();
	master_interact.msgQ_id = key_client;
	master_interact.sem_id = key_sem;
	master_interact.mem_seg = shmid_local;

	printf("%d | %d | %d | %d\n",(int)getpid(), key_client, key_sem, key_local );
	
	if(msgsnd(msqid_master,&master_interact,sizeof(master_msgbuf),0)== -1)
    {
        perror("Send Error 134: ");
    }

    while(1)
    {
    	if (msgrcv(msqid_client, &client_interact, sizeof(client_msgbuf), getpid(), 0) == -1)	// Client se interact
	    {
	       perror("Receive Error 109: ");
	    }

	    printf("\nClient %d Connected\n",client_interact.pid);

	    master_interact.type = TYPE2;
	    master_interact.atm_id = (int)getpid();
	   	master_interact.data = client_interact.pid;

	   	if(msgsnd(msqid_master,&master_interact,sizeof(master_msgbuf),0)== -1)
	    {
	        perror("Send Error 134: ");
	    }

	    if (msgrcv(msqid_master, &master_interact,sizeof(master_msgbuf), getpid(), 0) == -1)	// Master se interact
	    {
	       perror("Receive Error 109: ");
	    }
	    
	    if(strcmp(master_interact.msg,"NEW") == 0)
	    {
	    	printf("\nYou are Registered :) \n");
	    	
	    }
	    	
	    while(1)
	    {

		    if (msgrcv(msqid_client, &client_interact, sizeof(client_msgbuf), getpid(), 0) == -1)	// Client se interact
		    {
		       perror("Receive Error 109: ");
		    }

		    if(strcmp(client_interact.msg,"DEPOSIT") == 0)
		    {
		    	int money = client_interact.data;
		    	
		    	for(int i=1;i<150;i++)
		    	{
		    		if(local_mem->cid_amount[i][0] == 0)
		    		{
		    			local_mem ->cid_amount[i][0] = client_interact.pid;
				    	local_mem ->cid_amount[i][1] = money;
				    	strcpy(local_mem ->type[i],"DEPOSIT");

				    	time (&rawtime);
	  					timeinfo = localtime (&rawtime);
				    	strftime (local_mem ->time1[i],40,"%G%m%e%H%M%S",timeinfo);
				    	
				    	printf("\nClient %d : Successfully Deposit %d Rs. in your Account\n",client_interact.pid, money);
				    	break;
		    		}
		    	}

		    	for(int i=1;i<150;i++)
		    	{
		    		if(local_mem->cid_amount[i][0] == 0)
		    		{
		    			break;
		    		}
		    		else
		    		{
		    			printf("%d | %d | %s | ",local_mem->cid_amount[i][0],local_mem->cid_amount[i][1],local_mem->type[i]);

		    			for(int j=0;j<4;j++)
		    			{
		    				printf("%c",local_mem->time1[i][j]);
		    			}
		    			printf("-");
		    			for(int j=4;j<6;j++)
		    			{
		    				printf("%c",local_mem->time1[i][j]);
		    			}
		    			printf("-");
		    			for(int j=6;j<8;j++)
		    				printf("%c",local_mem->time1[i][j]);
		    			printf(" | ");

		    			for(int j=8;j<10;j++)
		    			{
		    				printf("%c",local_mem->time1[i][j]);
		    			}
		    			printf(":");

		    			for(int j=10;j<12;j++)
		    				printf("%c",local_mem->time1[i][j]);

		    			printf(":");

		    			for(int j=12;j<14;j++)
		    				printf("%c",local_mem->time1[i][j]);
		    			printf("\n");
		    		}
		    	}
		    	continue;
			}

			else if(strcmp(client_interact.msg,"WITHDRAW") == 0)
		    {
		    	int money = client_interact.data;
		    	int k = localconsis(money,client_interact.pid);
		    	
		    	if(k>=money)
		    	{
		    		for(int i=1;i<150;i++)
			    	{
			    		if(local_mem->cid_amount[i][0] == 0)
			    		{
			    			local_mem ->cid_amount[i][0] = client_interact.pid;
					    	local_mem ->cid_amount[i][1] = money;
					    	strcpy(local_mem ->type[i],"WITHDRAW");
					    	
					    	time (&rawtime);
		  					timeinfo = localtime (&rawtime);
					    	strftime (local_mem ->time1[i],40,"%G%m%e%H%M%S",timeinfo);

					    	strcpy(client_interact.msg,"Withdrawal Successful");
					    	client_interact.type = client_interact.pid;
							if(msgsnd(msqid_client,&client_interact,sizeof(client_msgbuf),0)== -1)
						    {
						        perror("Send Error 134: ");
						    }
					    	break;

			    		}
			    	}

			    	printf("\nClient %d : Successfully Withdraw %d Rs. from your Account\n",client_interact.pid, money);

			    	for(int i=1;i<150;i++)
			    	{
			    		if(local_mem->cid_amount[i][0] == 0)
			    		{
			    			break;
			    		}
			    		else
			    		{
			    			printf("%d | %d | %s | ",local_mem->cid_amount[i][0],local_mem->cid_amount[i][1],local_mem->type[i]);

			    			for(int j=0;j<4;j++)
			    			{
			    				printf("%c",local_mem->time1[i][j]);
			    			}
			    			printf("-");
			    			for(int j=4;j<6;j++)
			    			{
			    				printf("%c",local_mem->time1[i][j]);
			    			}
			    			printf("-");
			    			for(int j=6;j<8;j++)
			    				printf("%c",local_mem->time1[i][j]);
			    			printf(" | ");

			    			for(int j=8;j<10;j++)
			    			{
			    				printf("%c",local_mem->time1[i][j]);
			    			}
			    			printf(":");

			    			for(int j=10;j<12;j++)
			    				printf("%c",local_mem->time1[i][j]);

			    			printf(":");

			    			for(int j=12;j<14;j++)
			    				printf("%c",local_mem->time1[i][j]);
			    			printf("\n");
			    		}
			    	}
		    	}

				else
		    	{
		    		strcpy(client_interact.msg,"Balance Insufficient");
		    		client_interact.type = client_interact.pid;
	    			if(msgsnd(msqid_client,&client_interact,sizeof(client_msgbuf),0)== -1)
				    {
				        perror("Send Error 134: ");
				    }
		    	}

		    	
		    	
		    	continue;
	    	
	    	}

	    	else if(strcmp(client_interact.msg,"VIEW") ==0)
		    {
		    	master_interact.type = TYPE3;
			    master_interact.atm_id = (int)getpid();
			   	master_interact.data = client_interact.pid;

			   	int balance;
			   	if(msgsnd(msqid_master,&master_interact,sizeof(master_msgbuf),0)== -1)
			    {
			        perror("Send Error 134: ");
			    }

			    if (msgrcv(msqid_master, &master_interact, sizeof(master_msgbuf), getpid(), 0) == -1)	// Master se interact
			    {
			       perror("Receive Error 109: ");
			    }

			    if(strcmp(master_interact.msg,"DONE")==0)
			    {

			    	master = (struct global_master *)shmat(atoi(argv[2]),NULL,0);

			    	for(int i=1;i<50;i++)
			    	{
			    		if(master->client_id[i]==0)
			    		{
			    			break;
			    		}
			    		else if(master->client_id[i] == client_interact.pid)
			    		{
			    			balance = master->balance[i];
			    		}
			    		else
			    		{

			    		}
			    	}
			    }
			    char samay[10]; 
			    int rokda;

			    for(int i=1;i<50;i++)
			    {
			    	if(local_mem->globcid[i]==0 || local_mem->globcid[i] == client_interact.pid)
			    	{
			    		local_mem->final_amnt[i] = balance;
			    		rokda = balance;
			    		local_mem->globcid[i] = client_interact.pid;
			    		time (&rawtime);
						timeinfo = localtime (&rawtime);
				    	strftime (local_mem->timestamp[i],40,"%G%m%e%H%M%S",timeinfo);
				    	strcpy(samay,local_mem->timestamp[i]);
				    	break;

			    	}
			    }

			   memset(temp_cid_amount,0,sizeof(temp_cid_amount));
			    memset(temp_time1,'\0',sizeof(temp_time1));
			    memset(temp_type,'\0',sizeof(temp_type));

			    int k =1;
			    for(int i=1;i<150;i++)
			    {
			    	if (local_mem->cid_amount[i][0] == client_interact.pid && strcmp(local_mem->time1[i],samay)<0)
			    	{
			    		local_mem->cid_amount[i][0] = 0;
			    		local_mem->cid_amount[i][1] = 0;
			    		local_mem->type[i][0] = '\0';
			    		local_mem->time1[i][0] = '\0';
			    	}

			    	else
			    	{
			    		temp_cid_amount[k][0] =  local_mem->cid_amount[i][0];
			    		temp_cid_amount[k][1] = local_mem->cid_amount[i][1];
			    		strcpy(temp_type[k],local_mem->type[i]);
			    		strcpy(temp_time1[k],local_mem->time1[i]);
			    		k++;

			    		local_mem->cid_amount[i][0] = 0;
			    		local_mem->cid_amount[i][1] = 0;
			    		local_mem->type[i][0] = '\0';
			    		local_mem->time1[i][0] = '\0';

			    	}
			    }

			    for(int i=1;i<150;i++)
			    {
			    	local_mem->cid_amount[i][0]  = temp_cid_amount[i][0];
			    	local_mem->cid_amount[i][1] = temp_cid_amount[i][1];
			    	strcpy(local_mem->type[i],temp_type[i]);
			    	strcpy(local_mem->time1[i],temp_time1[i]);

			    }

			    int lol = shmdt((struct global_master *)master);

			    client_interact.data = rokda;
			    client_interact.type = client_interact.pid;
			    if(msgsnd(msqid_client,&client_interact,sizeof(client_msgbuf),0)== -1)
			    {
			        perror("Send Error 134: ");
			    }

			    
			    continue;
		    }

		    else if(strcmp(client_interact.msg,"LEAVE") == 0)
		    {
		    	master_interact.type = TYPE4;
			    master_interact.atm_id = (int)getpid();
			   	master_interact.data = client_interact.pid;

		    	if(msgsnd(msqid_master,&master_interact,sizeof(master_msgbuf),0)== -1)
			    {
			        perror("Send Error 134: ");
			    }
		    	break;
		    }

		}
	}

}