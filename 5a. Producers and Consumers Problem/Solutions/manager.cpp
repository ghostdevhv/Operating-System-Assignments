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

key_t key = 270, key1 = 271, key_sem=272;
vector<int> res_alloc[20];
int secondline[20],thirdline[20],semid;
int graph[12][12],parent[12],start_vertex,end_vertex;
bool vis[12],recstack[12];

void printcycle(int i)
{
	vector<int> v;
	while(i!=end_vertex){
		v.push_back(i);
		i=parent[i];
	}
	v.push_back(i);
	int mm=v.size();
	FILE *ptr=fopen("result.txt","a");
	for(int i=mm-1;i>=0;i--){
		printf("%d ",v[i]);
		fprintf(ptr, "%d ",v[i] );
	}
	fclose(ptr);
}

int checkcycle(int i)
{
	if(vis[i]==0){
		vis[i]=1;
		recstack[i]=1;
		for(int j=0;j<12;j++){
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
	for(int i=0;i<12;i++){
		parent[i]=-1;
		int f=(checkcycle(i));
		if(f) return f;
	}
	return 0;
}
struct sembuf sop;
int main()
{
	double p=0.7;
	memset(graph,0,sizeof(graph));
	int msqid,msqid1;
	vector<int> processes;
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget 73");
        exit(1);
    }

    if ((msqid1 = msgget(key1, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget79 ");
        exit(1);
    }
	if((semid = semget(key_sem,9,0666|IPC_CREAT)) == -1)
    {
        perror("msgget 84");
        exit(1);
    }
	semctl(semid,2,SETVAL,1);
	semctl(semid,3,SETVAL,10);
	semctl(semid,5,SETVAL,10);
	semctl(semid,0,SETVAL,1);
	semctl(semid,1,SETVAL,1);
    char temp1[1000],temp2[1000],temp[1000];

	FILE *ptr=fopen("matrix.txt","w");
	int abc=0;
    for(int i=1;i<=2;i++){
        for(int j=1;j<=10;j++)
            fprintf(ptr,"%d ",abc);
        fprintf(ptr, "\n");
    }
	fclose(ptr);
	strcpy(temp,"./producer");
    sprintf(temp1," %d ",key);
    strcat(temp,temp1);
   	sprintf(temp1,"%d ",key1);
    strcat(temp,temp1);
	sprintf(temp1,"%d ",key_sem);
 	strcat(temp,temp1);
	int pid;
	for(int i=1;i<=5;i++){
		usleep(500);
		strcpy(temp2,temp);
		sprintf(temp1,"%d ",i);
	 	strcat(temp2,temp1);
		if((pid=fork())==0) execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c",temp2, (void*)NULL);
		else processes.push_back(pid);
	}
	strcpy(temp,"./consumer");
    sprintf(temp1," %d ",key);
    strcat(temp,temp1);
   	sprintf(temp1,"%d ",key1);
    strcat(temp,temp1);
	sprintf(temp1,"%d ",key_sem);
 	strcat(temp,temp1);
	for(int i=6;i<=10;i++){
		usleep(500);
		strcpy(temp2,temp);
		sprintf(temp1,"%d ",i);
	 	strcat(temp2,temp1);
		if((pid=fork())==0) execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c",temp2, (void*)NULL);
		else processes.push_back(pid);
	}
    //strcat(temp,"Producer");
    //strcat(temp,"Consumer");
	char linelo[1000];
	int i,j,k=1,mat[3][11];
	while(1)
	{
		fprintf(stderr, "%s\n", "trying to get lock on file");
		sop.sem_num=2;
        sop.sem_op=-1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // get lock on file matrix.txt
        printf("got lock on file\n");
		FILE *ptr = fopen("matrix.txt","r");
		for(i=1;i<=2;i++)
	        for(j=1;j<=10;j++)
	            fscanf(ptr,"%d",&mat[i][j]);
		fclose(ptr);
		sop.sem_num=2;
        sop.sem_op=1;
        sop.sem_flg=0;
        semop(semid,&sop,1);  // release lock on file matrix.txt
        printf("release lock on file 88\n");
		memset(graph,0,sizeof(graph));
		for(i=1;i<=2;i++){
			for(j=1;j<=10;j++){
				//printf("mat[%d][%d]=%d\n",i,j,mat[i][j]);
				if(mat[i][j]==1){
					printf("edge %d - %d\n",j+1,i-1);graph[j+1][i-1]=1;
				}
				else if(mat[i][j]==2){
					printf("edge %d - %d\n",i-1,j+1);graph[i-1][j+1]=1;
				}
			}
		}
		// for(int i=0;i<12;i++){
		// 	for(int j=0;j<12;j++) if(graph[i][j]) printf("%d -> %d\n",i,j);
		// }
		int res=isCyclic();
		if(res){
			printcycle(res); printf("cycle found\n");
			int xx=semctl(semid,7,GETVAL,0);
			int yy=semctl(semid,8,GETVAL,0);
			printf("insertion=%d\ndeletion=%d\n",xx,yy);
			FILE *ptr=fopen("result.txt","a");
			fprintf(ptr, "%lf insertion=%d deletion=%d\n",p,xx,yy);
			fclose(ptr);
			printf("Killing all processes\n");
			int mm=processes.size();
			for(int i=0;i<mm;i++) kill(processes[i],SIGKILL);
			printf("All processess killed Successfully\nExiting\n");
			return 0;
		}
		printf("manager going to sleep for 2 seconds %d time\n",k++);
		int xx=semctl(semid,7,GETVAL,0);
		int yy=semctl(semid,8,GETVAL,0);
		printf("insertion=%d\ndeletion=%d\n",xx,yy);
		sleep(2);
		printf("awake\n");
		/// Graph me store hai sab
		// Yahan cycle detect krne ka code likhna padega

	}
}
