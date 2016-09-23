#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <wait.h>

using namespace std;

//#define _GNU_SOURCE
#define MAX_CLIENT 20
#define MAX_CHAT_ID 50
#define SER_TYPE 1
#define MAXC MAX_CLIENT * (MAX_CHAT_ID + 10)

char force1[4000];

key_t key = 139;

typedef struct my_msgbuf
{
    long type;
    char mtext[4000];
    pid_t pid;
}my_msgbuf;



int main()
{
	char check[4000];
	char clientMSG[4000];
	printf("Pleses type \"couple\" to connect to server\n");
	scanf("%s",check);
	my_msgbuf msg,buff;
	int msqid;
	struct msqid_ds info;

	while(strcmp(check,"couple")!=0)
	{
		printf("Pleses type \"couple\" to connect to server\n");
		scanf("%s",check);
	}

	msqid = msgget(key,IPC_CREAT|0666);		//message queue from client to server
	msg.pid=getpid();
	
	bool load = true;
	bool check_couple  = true;
	bool cntre = false;
	while(1)
	{

		strcpy(msg.mtext,"NEW");
		msg.type = 1;
		if(msgsnd(msqid,&msg,sizeof(my_msgbuf),0)==-1)
		{
			perror("Send Error : ");
		}
		int a = fork();

		if(a==0)
		{
			while(1)
			{
				while(load==1 && (msgrcv(msqid, &buff, sizeof(my_msgbuf), msg.pid,0) != -1))
				{

					printf("%s\n",buff.mtext);
					
				}
			}
		}

		else if (a>0)
		{
			while(1)
			{
				char force[4000];
				char select[4000];
				char asdf[1000];
				//int lol;
				if(check_couple)
				{
					getchar();
					scanf("%[^\n]", select);
				}
				else
				{
					//printf("check_couple false\n");
					if(cntre)
					{
						//printf("cntre true\n");
						strcpy(select,asdf);
						cntre = false;
					}
					else
					{
						//printf("cntre false\n");
						getchar();
						scanf("%[^\n]", select);
						
						if(strcmp(select,"couple")==0)
						{
							load = true;
							check_couple = true;
							break;
						}
					}
				}
				//printf("%s\n",select );
				strcpy(force,select);
		        strcat(force,"\n");

			 	if (strcmp(select,"uncouple")==0)
				{
					load = false;
					msg.type = 1;
				 	strcpy(msg.mtext,select);
					msgsnd(msqid, &msg, sizeof(my_msgbuf), 0);
					//printf("-----------------------\n");

					getchar();
					scanf("%[^\n]", asdf);
					if(strcmp(asdf,"couple")==0)
					{
						load = true;
						check_couple = true;
						break;
					}
					else if (strcmp(asdf,"exit")==0)
					{
						//printf("Ho\n");
						kill(a,SIGKILL);
						exit(0);
					}
					else
					{
						//printf("asdf     %s\n",asdf);
						check_couple = false;
						cntre = true;
						continue;
					}
				}

				else if (strcmp(select,"exit")==0)
				{
					if(check_couple)
					{
						//printf("%d\n",load);
						kill(a,SIGKILL);
						msg.type = 1;
					 	strcpy(msg.mtext,"uncouple");
						msgsnd(msqid, &msg, sizeof(my_msgbuf), 0);
						exit(0);
					}
					else
					{
						kill(a,SIGKILL);
						printf("You are disconnected from the server\n");
						exit(0);
					}
				}
				
				else
				{

					int flag = 1;
					int status;
					int bufsize = 4000;
		            int position = 0;
		            char **tokens = (char **) malloc(sizeof(char *)*bufsize);
		            char *token;
		            int tt=getpid();
		            token = strtok(select," ");

		            while(token!=NULL)
		            {
		                tokens[position] = token;
		                position++;
		                token = strtok(NULL," ");
		            }
		            tokens[position] = (char*)0;

		            pid_t pid;

					if(load)
					{
						//printf("load true\n");
						load = true;
						check_couple = true;

						if((pid=fork())<0)
						{
							perror("fork"); //exit(EX_OSERR);
						}

						if(pid==0)
					    {
					    	int fd = creat("/home/himanshu/Documents/temp234.txt",0666);
							dup2(fd,1);
					    	close(fd);
					        if(execvp(tokens[0], tokens)<0)
					        {

					        }
				    	}

				    	else
				    	{
				    		while(wait(&status)!=pid);
				    		FILE * fp;
						    char * line = NULL;
						    size_t len = 0;
						    ssize_t read;
				    		fp = fopen("/home/himanshu/Documents/temp234.txt","r");
				    		if(fp==NULL)
				    			exit(EXIT_FAILURE);

				    		while ((read = getline(&line, &len, fp)) != -1) 
				    		{
						        printf("%s", line);
						        strcat(force,line);
			   				}

		   					fclose(fp);
						    if (line)
						        free(line);
							msg.type = SER_TYPE;
						 	strcpy(msg.mtext,force);
						 	msg.pid = getpid();
							msgsnd(msqid, &msg, sizeof(my_msgbuf), 0);
						}

					}

					else
					{
						//printf("load false\n");
						check_couple = false;
						load = false;
						if((pid=fork())<0)
						{
							perror("fork"); //exit(EX_OSERR);
						}

						if(pid==0)
						{
							if(execvp(tokens[0],tokens)<0)
								perror("execvp");
						}
						else
						{
							while(wait(&status)!=pid);
						}
					}
				}
			}
		}
		if(load==1)
			continue;
	}
	return 0;
}
