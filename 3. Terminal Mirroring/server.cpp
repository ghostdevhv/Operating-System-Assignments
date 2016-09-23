#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


using namespace std;

#define MAX 4000

key_t key=139;

int compare(char const *s1, char const *s2){
    int i = 0;
    for(i = 0; s2[i]; i++){
        if(s1[i] != s2[i])
            return -1;
    }
    return 0;
}


typedef struct my_msgbuf
{
    long type;
    char mtext[4000];
    pid_t pid;
}my_msgbuf;

pair<int,int> client_name[MAX];


void handler(int s){
 struct msqid_ds buf;
 int msqid;

 msqid = msgget(key, 0666);

 msgctl(msqid,IPC_RMID,&buf);
 printf("\n****Terminating Server****\n");
 exit(0);
}


int main()
{
    struct my_msgbuf buff,msg;
    int msqid,client_no,total_clients=0;
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    char t2[10];
    signal(SIGINT, handler);
    while(1)
    {
        int i;
        if (msgrcv(msqid, &buff, sizeof(my_msgbuf), 1, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }

        if(compare(buff.mtext, "NEW") == 0)
        {
            //
            //printf("AA gya");
            total_clients++;
            for(i=0;i<MAX;i++)
                if(client_name[i].first==0){
                    client_name[i].first=i+1;
                    break;
                }
            client_name[i].second=buff.pid;
            sprintf(t2,"%d",i+1);
            strcpy(buff.mtext,"Congratulations. Your terminal ID is ");
            strcat(buff.mtext,t2);
            strcat(buff.mtext,"\0");
            buff.type=buff.pid;
            if(msgsnd(msqid,&buff,sizeof(my_msgbuf),0) == -1)
            {
                perror("msgsnd: msgsnd failed");
                exit(1);
            }
            strcpy(buff.mtext,"Terminal ");
            strcat(buff.mtext,t2);
            strcat(buff.mtext," connected to server\0");
            for(int i=0;i<total_clients;i++)
            {
                buff.type=client_name[i].second;
                if(msgsnd(msqid,&buff,sizeof(my_msgbuf),0) == -1)
                {
                    perror("msgsnd: msgsnd failed");
                    exit(1);
                }
            }
        }
        else if(strcmp(buff.mtext,"uncouple")==0){
            int t1=buff.pid;
            int sendersid;
            for(i=0;i<total_clients;i++){
                if(client_name[i].second==t1){
                    sendersid= client_name[i].first;
                    client_name[i].first=0;
                    break;
                }
            }
            strcpy(msg.mtext,"You have been disconnected");
            msg.type=t1;
            if(msgsnd(msqid,&msg,sizeof(my_msgbuf),0) == -1)
            {
                perror("msgsnd: msgsnd failed");
                exit(1);
            }
            strcpy(msg.mtext,"Terminal ");
            sprintf(t2,"%d ",sendersid);
            strcat(msg.mtext,t2);
            strcat(msg.mtext,"disconnected from the server");
            for(i=0;i<total_clients;i++){
                if(client_name[i].first==0) continue;
                msg.type=client_name[i].second;
                if(msgsnd(msqid,&msg,sizeof(my_msgbuf),0) == -1)
                {
                    perror("msgsnd: msgsnd failed");
                    exit(1);
                }
            }
            total_clients--;
        }
        else
        {
            printf("here\n");
            int i,sendersid;
            for(i=0;i<total_clients;i++){
                //printf("%d-------%d -------- %d\n",client_name[i].first,client_name[i].second,buff.pid);

                if(client_name[i].second==buff.pid){
                    sendersid=client_name[i].first;
                    break;
                }
            }
            printf("%d\n",sendersid);
            strcpy(msg.mtext,"Terminal ");
            sprintf(t2,"%d: ",sendersid);
            strcat(msg.mtext,t2);
            strcat(msg.mtext,buff.mtext);
            for(i=0;i<total_clients;i++){
                printf("%d\n", client_name[i].first);
                msg.type=client_name[i].second;
                if(client_name[i].first==sendersid) continue;
                if(msgsnd(msqid,&msg,sizeof(my_msgbuf),0) == -1)
                {
                    perror("msgsnd: msgsnd failed");
                    exit(1);
                }
            }

        }
    }

    return 0;
}
