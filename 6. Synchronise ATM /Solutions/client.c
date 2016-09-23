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

typedef struct atm_msgbuf	// ATM k saath Msq Queue
{
	long type;
	int pid;
	char msg[100];
	int data;

}atm_msgbuf;

struct atmlocator
{
	int atm_id[50];
	int msgQ_id[50];
	int sem_id[50];
	int mem_seg[50];
};

int main()
{
	key_t key_atmloc = 300;
	key_t key_sem;
	key_t key_atm;

	char command[100],aagewala[100],atmno[2];
	int atm_no;

	int shmid_atmloc, semid_lock, msqid_atm;

	if((shmid_atmloc = shmget(key_atmloc, MAX, IPC_CREAT|0666)) == -1) // ATM Locator Shared File
	{
		perror("shmget ---> ATM_Locator");
		exit(1);
	}

	struct atmlocator* atm_loc;
	atm_loc = (struct atmlocator *)shmat(shmid_atmloc,NULL,0); // Now Client can access ATM Locator shared memory space

	struct sembuf sop;
    struct atm_msgbuf atm_inter;

	for(int i=1;i<10;i++)
	{
		if(atm_loc->atm_id[i]!=0)
		{
			printf("%d | %d | %d | %d\n",atm_loc->atm_id[i],atm_loc->msgQ_id[i], atm_loc->sem_id[i], atm_loc->mem_seg[i]);
			key_sem = atm_loc->sem_id[i];
		}
		else 
			break;
	}

	

	if((semid_lock = semget(key_sem, atm_loc->atm_id[0], 0666|IPC_CREAT)) == -1) // Semaphore for atm lock
    {
        perror("semget --> Atm_Lock"); 
        exit(1);
    }

    labelA: while(1)
    {
    	printf("\nTotal number of ATMs are %d\n", atm_loc->atm_id[0]);
    	printf("\nPlease enter \"ENTER ATMx\" where x is the number of ATM [ x must be less than equal to total number of ATMs ]\n");
		scanf("%s",command);
		scanf("%s",aagewala);
		if(strcmp(command,"ENTER") ==0)
		{
			int k=0;
			for(int i=3;i<strlen(aagewala);i++)
			{	
				atmno[k] = aagewala[i];
				k++;
			}
			atmno[k] = '\0';
			atm_no = atoi(atmno);
			printf("%d\n",atm_no );
			if(atm_no > atm_loc->atm_id[0])
			{

			}
			else 
			{
				sop.sem_num = atm_no-1;
				sop.sem_op = -1;
				sop.sem_flg = IPC_NOWAIT;
				if(semop(semid_lock,&sop,1) == -1)
				{
					printf("ATM%d Occupied\n",atm_no);
					continue;
				}
				else
				{
					printf("Welcome Client %d\n",(int)getpid());
					break;
				}
			}
		}
		printf("Invalid Command | Please enter the right command\n");
    }


   	if ((msqid_atm = msgget(atm_loc->msgQ_id[atm_no], 0666 | IPC_CREAT)) == -1)	// Create a message queue with ATM
	{
	    perror("msgget");
	    exit(1);
	}

	atm_inter.type = atm_loc->atm_id[atm_no];
	atm_inter.pid = (int)getpid();
	
	if(msgsnd(msqid_atm,&atm_inter,sizeof(atm_msgbuf),0)== -1) // Me connect ho gya
    {
        perror("Send Error 134: ");
        goto labelA;
    }

    while(1)
	{
		printf("\n ATM Options:\n");
		printf("\n\"WITHDRAW x\" --> Withdraw x rs. from your Account");
		printf("\n\"DEPOSIT x\" --> Deposit x rs. in your Account");
		printf("\n\"VIEW\" --> View total cash available in your account");
		printf("\n\"LEAVE\" --> Leave ATM");
		printf("\n\n Please enter the command: ");
		scanf("%s",command);

		if(strcmp(command,"WITHDRAW")!=0 && strcmp(command,"ADD")!=0 && strcmp(command,"DEPOSIT")!=0 && strcmp(command,"VIEW")!=0 && strcmp(command,"LEAVE")!=0)
		{
			printf("\n Error : Invalid Command.  Please try again\n");
			continue;
		}

		if(strcmp(command,"WITHDRAW")==0)
		{
			int money;

			scanf("%d",&money);

			atm_inter.type = atm_loc->atm_id[atm_no];
			atm_inter.pid = (int)getpid();
			atm_inter.data = money;
			strcpy(atm_inter.msg,"WITHDRAW");

			if(msgsnd(msqid_atm,&atm_inter,sizeof(atm_msgbuf),0)== -1)
		    {
		        perror("Send Error 134: ");
		    }

		    if (msgrcv(msqid_atm, &atm_inter, sizeof(atm_msgbuf), getpid(), 0) == -1)	// Client se interact
		    {
		       perror("Receive Error 109: ");
		    }

		    printf("%s\n",atm_inter.msg);
		    continue;
		}

		else if(strcmp(command,"DEPOSIT")==0)
		{
			int money;
			scanf("%d",&money);

			atm_inter.type = atm_loc->atm_id[atm_no];
			atm_inter.pid = (int)getpid();
			atm_inter.data = money;
			strcpy(atm_inter.msg,"DEPOSIT");

			if(msgsnd(msqid_atm,&atm_inter,sizeof(atm_msgbuf),0)== -1)
		    {
		        perror("Send Error 134: ");
		    }

		    printf("%s\n", atm_inter.msg);
		    continue;

		}

		else if (strcmp(command,"VIEW")==0)
		{
			atm_inter.type = atm_loc->atm_id[atm_no];
			atm_inter.pid = (int)getpid();
			strcpy(atm_inter.msg,"VIEW");
			atm_inter.data = atm_no;

			if(msgsnd(msqid_atm,&atm_inter,sizeof(atm_msgbuf),0)== -1)
		    {
		        perror("Send Error 134: ");
		    }

		    if (msgrcv(msqid_atm, &atm_inter, sizeof(atm_msgbuf), getpid(), 0) == -1)	// Client se interact
		    {
		       perror("Receive Error 109: ");
		    }

		    printf("You have %d Rs. in your Account No. %d\n",atm_inter.data,(int)getpid());
		    continue;
		}

		else if(strcmp(command,"LEAVE")==0)
		{
			sop.sem_num = atm_no-1;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			semop(semid_lock,&sop,1);
			printf("\n Good Bye Client %d. Have a Nice Day! :)\n",(int)getpid());

			atm_inter.type = atm_loc->atm_id[atm_no];
			atm_inter.pid = (int)getpid();
			strcpy(atm_inter.msg,"LEAVE");
			atm_inter.data = atm_no;

			if(msgsnd(msqid_atm,&atm_inter,sizeof(atm_msgbuf),0)== -1)
		    {
		        perror("Send Error 134: ");
		    }

			goto labelA;

		}
	}

}