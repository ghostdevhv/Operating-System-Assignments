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


struct sembuf sop,sop1;
int semid1,semid2;
key_t key_sem1, key_sem2;
char oneD;
int process_num;
char train_num[100][100];

#define token_delimiters " "

void updatematrix(int val, int col_num)
{
	vector<int> vec[1000];
	FILE *ptr;
	sop1.sem_num = 1;
	sop1.sem_op = -1;
	sop1.sem_flg = 0;
	semop(semid1,&sop1,1);	// Request for File Lock

	printf("Got File Access :P \n");

	ptr = fopen("matrix.txt","r");
	int lola = 0;
	int k = 0;
	char linelo[1000];
	if (ptr!= NULL)
	{
	    char line[256];
	    while (fgets(line, sizeof line, ptr) != NULL)
	    {
	    	if(lola>=2)
	    	{
	    		strcpy(linelo, line);

	    		int position = 0;
                char **tokens = (char**)malloc(sizeof(char*) * 1000);
                char *token;
                token = strtok(linelo,token_delimiters);

                while(token!=NULL)
                {
                    tokens[position] = token;
                    position++;
                    token = strtok(NULL,token_delimiters);
                }
                tokens[position] = NULL;
                strcpy(train_num[k],tokens[0]);
                for(int i=1;i<position;i++)
                {
                	vec[k].push_back(atoi(tokens[i]));
                	
                }
                k++;
	    		lola++;
	    	}
	    	else
	    	{
	    		lola++;
	    	}
	    }
	    fclose(ptr);
	}

	vec[process_num-1][col_num] = val;
	// remove("matrix.txt");
	ofstream myfile;
    myfile.open("matrix.txt");
    
    myfile<< "Train" <<setw(13)<<"East" <<setw(13)<<setw(13)<<"West" <<setw(13)<<"North" <<setw(13)<<"South\n\n"; 

    for(int i=0;i<k-1;i++)
    {
    	myfile<<train_num[i];
    	myfile<<setw(13)<<vec[i][0]<<setw(13)<<vec[i][1]<<setw(13)<<vec[i][2]<<setw(13)<<vec[i][3];
    	myfile<<"\n";
    }

    myfile<<train_num[k-1];
	myfile<<setw(13)<<vec[k-1][0]<<setw(13)<<vec[k-1][1]<<setw(13)<<vec[k-1][2]<<setw(13)<<vec[k-1][3];

    myfile.close();

    sop1.sem_num = 1;
	sop1.sem_op =  1;
	sop1.sem_flg = 0;
	semop(semid1,&sop1,1);	// Release File Lock

	printf("File me write ho gya :)\n");
	// return;
}

int main(int argc, char* argv[])
{
	FILE *trauma;
	trauma = fopen("store_pid.txt","a");
	fprintf(trauma, "%d %d\n",atoi(argv[4]),(int)getpid());
	fclose(trauma);	
	key_sem1 = atoi(argv[1]); //For matrix and Junction lock
	key_sem2 = atoi(argv[2]); // For NEWS lock
	oneD = argv[3][0]; // Which direction ? 
	process_num = atoi(argv[4]); // Process Number

	if((semid1 = semget(key_sem1,2,0666|IPC_CREAT)) == -1) 
    {
        perror("semget");
        exit(1);
    }

    if((semid2 = semget(key_sem2,4,0666|IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(1);
    }

	if(oneD=='E')
	{
		printf("Train <%d>: East train Started\n",(int)getpid());
		
		printf("Train <%d>: requests for East-Lock\n",(int)getpid());
		updatematrix(1,0);
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for East Lock
		printf("Train <%d>: acquires East-Lock\n",(int)getpid());
		updatematrix(2,0);

		printf("Train <%d>: requests for North-Lock\n",(int)getpid());
		updatematrix(1,2);
		sop.sem_num = 2;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for North Lock
		printf("Train <%d>: acquires North-Lock\n",(int)getpid());
		updatematrix(2,2);

		printf("Train <%d>: requests for Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op = -1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Request for Junction Lock
		printf("Train <%d>: acquires Junction-Lock; Passing Junction\n",(int)getpid());

		sleep(2);
		printf("Train <%d>: releases Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op =  1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Releases Junction Lock

		sop.sem_num = 2;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases North Lock
		printf("Train <%d>: releases North-Lock\n",(int)getpid());
		updatematrix(0,2);

		sop.sem_num = 0;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases East Lock
		printf("Train <%d>: releases East-Lock\n",(int)getpid());
		updatematrix(0,0);	

	}

	else if(oneD == 'W')
	{
		printf("Train <%d>: West train Started\n",(int)getpid());
		
		printf("Train <%d>: requests for West-Lock\n",(int)getpid());
		updatematrix(1,1);
		sop.sem_num = 1;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for West Lock
		printf("Train <%d>: acquires West-Lock\n",(int)getpid());
		updatematrix(2,1);
		
		printf("Train <%d>: requests for South-Lock\n",(int)getpid());
		updatematrix(1,3);
		sop.sem_num = 3;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for North Lock
		printf("Train <%d>: acquires South-Lock\n",(int)getpid());
		updatematrix(2,3);

		printf("Train <%d>: requests for Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op = -1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Request for Junction Lock
		printf("Train <%d>: acquires Junction-Lock; Passing Junction\n",(int)getpid());

		sleep(2);
		printf("Train <%d>: releases Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op =  1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Releases Junction Lock

		sop.sem_num = 3;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases South Lock
		printf("Train <%d>: releases South-Lock\n",(int)getpid());
		updatematrix(0,3);

		sop.sem_num = 1;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases West Lock
		printf("Train <%d>: releases West-Lock\n",(int)getpid());
		updatematrix(0,1);
	}

	else if(oneD == 'N')
	{
		printf("Train <%d>: North train Started\n",(int)getpid());
		
		printf("Train <%d>: requests for North-Lock\n",(int)getpid());
		updatematrix(1,2);
		sop.sem_num = 2;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for North Lock
		updatematrix(2,2);
		printf("Train <%d>: acquires North-Lock\n",(int)getpid());

		printf("Train <%d>: requests for West-Lock\n",(int)getpid());
		updatematrix(1,1);
		sop.sem_num = 1;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for West Lock
		updatematrix(2,1);
		printf("Train <%d>: acquires West-Lock\n",(int)getpid());

		printf("Train <%d>: requests for Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op = -1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Request for Junction Lock
		printf("Train <%d>: acquires Junction-Lock; Passing Junction\n",(int)getpid());

		sleep(2);
		printf("Train <%d>: releases Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op =  1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Releases Junction Lock

		printf("Train <%d>: releases West-Lock\n",(int)getpid());
		sop.sem_num = 1;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases West Lock
		updatematrix(0,1);

		printf("Train <%d>: releases North-Lock\n",(int)getpid());
		sop.sem_num = 2;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases North Lock
		updatematrix(0,2);
	}

	else
	{

		printf("Train <%d>: South train Started\n",(int)getpid());
		
		printf("Train <%d>: requests for South-Lock\n",(int)getpid());
		updatematrix(1,3);
		sop.sem_num = 3;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for South Lock
		updatematrix(2,3);
		printf("Train <%d>: acquires South-Lock\n",(int)getpid());

		printf("Train <%d>: requests for East-Lock\n",(int)getpid());
		updatematrix(1,0);
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Request for East Lock
		updatematrix(2,0);
		printf("Train <%d>: acquires East-Lock\n",(int)getpid());

		printf("Train <%d>: requests for Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op = -1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Request for Junction Lock
		printf("Train <%d>: acquires Junction-Lock; Passing Junction\n",(int)getpid());

		sleep(2);
		printf("Train <%d>: releases Junction-Lock\n",(int)getpid());
		sop1.sem_num = 0;
		sop1.sem_op =  1;
		sop1.sem_flg = 0;
		semop(semid1,&sop1,1);	// Releases Junction Lock

		printf("Train <%d>: releases East-Lock\n",(int)getpid());
		sop.sem_num = 0;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases East Lock
		updatematrix(0,0);

		printf("Train <%d>: releases South-Lock\n",(int)getpid());
		sop.sem_num = 3;
		sop.sem_op =  1;
		sop.sem_flg = 0;
		semop(semid2,&sop,1);	// Releases South Lock
		updatematrix(0,3);
	}

	getchar();

}