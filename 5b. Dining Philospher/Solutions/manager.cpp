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

key_t key_sem1 = 240, key_sem2 = 241;
int semid1,semid2;
char seq[100];
int length;
struct sembuf sop1;
char train_num[100][100];
int flag = 0;
vector<int> res_alloc[20];
int graph[20][20],parent[40],start_vertex,end_vertex;
bool vis[40],recstack[40];
vector<int> print;


void printcycle(int i)
{
	printf("\n\n");
	printf("------------------------------------\nSystem Deadlocked\n");
	while(i!=end_vertex){
		print.push_back(i);
		i=parent[i];
	}
	print.push_back(i);
	for(int i=print.size()-1;i>0;i--)
		printf("%d --> ",print[i]);
	printf("%d\n\n",print[0]);

	int pid[length+1];
	int d;
	char pid1[1000];
	FILE *ptr = fopen("store_pid.txt","r");
	for(int i=0;i<length;i++)
	{
		fscanf(ptr,"%d",&d);
		fscanf(ptr,"%s",pid1);
		pid[d-1] = atoi(pid1);
	}
	fclose(ptr);

    int last;
    char lastc;
    int flag1 = 0;
    for(int i=print.size()-1;i>0;i--)
	{

		flag1 = 0;
		if(length>print[i])
		{
			printf("Train <%d> from ",pid[print[i]]);
			if(seq[print[i]]=='N')
			{
				printf("North ");
			}
			else if(seq[print[i]]=='E')
			{
				printf("East ");
			}
			else if(seq[print[i]]=='S')
			{
				printf("South ");
			}
			else
			{
				printf("West ");
			}
			printf("is waiting for ");
			flag1 = 1;
			for(int j = i-1;j>=0;j--)
			{
				
				if(length>print[j])
				{
					printf("Train <%d> from ",pid[print[j]]);
					if(seq[print[j]]=='N')
					{
						printf("North --> \n");
					}
					else if(seq[print[j]]=='E')
					{
						printf("East --> \n");
					}
					else if(seq[print[j]]=='S')
					{
						printf("South -->  \n");
					}
					else
					{
						printf("West --> \n");
					}
					last = pid[print[j]];
					lastc = seq[print[j]];
					break;
				}
			}
		}
	}

	if(flag1==0)
	{
		printf("Train <%d> from ",last);
		if(lastc=='E')
		{
			printf("East is waiting for ");
		}
		else if(lastc=='W')
		{
			printf("West is waiting for ");
		}
		else if(lastc=='N')
		{
			printf("North is waiting for ");
		}
		else
		{
			printf("South is waiting for ");
		}
	}
	for(int i=print.size()-1;i>0;i--)
	{
		if(length>print[i])
		{
			printf("Train <%d> from ",pid[print[i]]);
			if(seq[print[i]]=='N')
			{
				printf("North\n");
			}
			else if(seq[print[i]]=='E')
			{
				printf("East\n");
			}
			else if(seq[print[i]]=='S')
			{
				printf("South\n");
			}
			else
			{
				printf("West\n");
			}
			break;
		}
	}
	// exit(1);
}

int checkcycle(int i)
{
	if(vis[i]==0){
		vis[i]=1;
		recstack[i]=1;
		for(int j=0;j<length+4;j++){
			if(graph[i][j]){
				if(vis[j]==0){
					parent[j]=i;
					int f=(checkcycle(j));
					if(f) return f;
				}
				else if(recstack[j]){
					end_vertex=j; return i;
				}
			}
		}
	}
	recstack[i]=false;
	return 0;
}

int isCyclic()
{
	memset(vis,0,sizeof(vis));
	memset(recstack,0,sizeof(recstack));
	for(int i=0;i<length+4;i++){
		parent[i]=-1;
		int f=(checkcycle(i));
		if(f) return f;
	}
	return 0;
}



int checkdeadlock()
{
    vector<int> vec[1000];

	sop1.sem_num = 1;
    sop1.sem_op = -1;
    sop1.sem_flg = 0;
    semop(semid1,&sop1,1);  // Request for File Lock
    printf("Got File Access\n");

    FILE *ptr;
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

    memset(graph,'\0',sizeof(graph));
    int asdf = vec[0].size();
    // printf("%d\n",asdf);
    for(int i=0;i<k;i++)
    {
        for(int j=0;j<asdf;j++)
        {
            if(vec[i][j] == 2)
            {
                graph[k+j][i] = 1;
            }
            else if(vec[i][j] == 1)
            {
                graph[i][k+j] = 1;
            }
        }
    }

    // for(int i=0;i<length+4;i++)
    // {
    //     for(int j=0;j<length+4 ;j++)
    //         printf("%d ",graph[i][j]);
    //     printf("\n");
    // }

    int res=isCyclic();
    if(res){
        printcycle(res); 
        flag = 1;
    }

    sop1.sem_num = 1;
    sop1.sem_op =  1;
    sop1.sem_flg = 0;
    semop(semid1,&sop1,1);  // Release File Lock
    printf("Release File lock\n\n");

    return flag;

}

int main(int argc, char* argv[])
{
	remove("store_pid.txt");
	srand(time(NULL));
	FILE *ptr;
	int b;
	double prob = atof(argv[1]);

	ptr = fopen("sequence.txt","r");

	if (ptr!= NULL)
	{
	    char line[256];
	    while (fgets(line, sizeof line, ptr) != NULL)
	    {
	    	strcpy(seq,line);
	    }
	    fclose(ptr);
	}
    char tempo[1000];
	length = strlen(seq);
    remove("matrix.txt");
    ofstream myfile;
    myfile.open("matrix.txt");
    
    myfile<< "Train" <<setw(13)<<"East" <<setw(13)<<setw(13)<<"West" <<setw(13)<<"North" <<setw(13)<<"South\n\n"; 
    
    for(int i=0;i<strlen(seq)-1;i++)
    {
        strcpy(tempo,"( ");
        tempo[strlen(tempo)-1] = seq[i];
        tempo[strlen(tempo)] = '\0';
        strcat(tempo,"T");
        strcat(tempo,")");
        myfile <<tempo<<setw(13)<<"0"<<setw(13)<<"0" <<setw(13)<<"0" <<setw(13)<<"0\n";
    }

    strcpy(tempo,"( ");
    tempo[strlen(tempo)-1] = seq[strlen(seq)-1];
    tempo[strlen(tempo)] = '\0';
    strcat(tempo,"T");
    strcat(tempo,")");
    myfile <<tempo<<setw(13)<<"0"<<setw(13)<<"0" <<setw(13)<<"0" <<setw(12)<<"0";

    myfile.close();
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

    semctl(semid1,0,SETVAL,1);
    semctl(semid1,1,SETVAL,1);
    semctl(semid2,0,SETVAL,1);
    semctl(semid2,1,SETVAL,1);
    semctl(semid2,2,SETVAL,1);
    semctl(semid2,3,SETVAL,1);

    int count = 0;
    while(1)
    {
    	double p = ((double) rand() / (RAND_MAX));
        // printf("%lf\n",p);
    	if(p<=prob)
    	{
    		if(count==0)
            {
                continue;
            }
            else
            {
                int k = checkdeadlock();
                if(k==1)
                {
                    
                    break;
                }
            }
    	}
    	else
    	{
            count++;
    		if(strlen(seq) >= count)
    		{
                char temp[1000], temp1[1000];
                strcpy(temp,"./train ");
                sprintf(temp1,"%d ",key_sem1);
                strcat(temp,temp1);
                sprintf(temp1,"%d ",key_sem2);
                strcat(temp,temp1);
                sprintf(temp1,"%c ",seq[count-1]);
                strcat(temp,temp1);
                sprintf(temp1,"%d",count);
                strcat(temp,temp1);
    			if((b==fork())==0)
    			{
    				execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c",temp, (void*)NULL);
    			}
    			else
    			{
                    
    			}
    		}
    		else
    			break;
    	}
    }

    while(1)
    {
    	sleep(1);
    	int k = checkdeadlock();
        if(k==1)
        {
        	
        	break;
        }



    }

    ptr = fopen("store_pid.txt","r");
    int arr,brr;
    while(ptr!=NULL)
    {
    	fscanf(ptr,"%d %d",&arr,&brr);
    	kill(brr,SIGKILL);
    }
    fclose(ptr);
  
}